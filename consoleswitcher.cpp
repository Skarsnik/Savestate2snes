#include "consoleswitcher.h"
#include "ui_consoleswitcher.h"

Q_LOGGING_CATEGORY(log_consoleSwitcher, "ConsoleSwitcher")

#define sDebug() qCDebug(log_consoleSwitcher())

#define B_A_BITMASK 0x0080
#define B_B_BITMASK 0x8000
#define B_Y_BITMASK 0x4000
#define B_X_BITMASK 0x0040
#define B_START_BITMASK 0x1000
#define B_SELECT_BITMASK 0x2000
#define B_UP_BITMASK 0x0800
#define B_DOWN_BITMASK 0x0400
#define B_LEFT_BITMASK 0x0200
#define B_RIGHT_BITMASK 0x0100
#define B_L_BITMASK 0x0020
#define B_R_BITMASK 0x0010

#define HEXDUMPSTR "hexdump -v -e '32/1 \"%02X\" \"\\n\"' /dev/input/by-path/platform-twi.1-event-joystick"

#define SNES_CLASSIC_IP "169.254.13.37"
//#define SNES_CLASSIC_IP "192.168.0.204"

ConsoleSwitcher::ConsoleSwitcher(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConsoleSwitcher)
{
    ui->setupUi(this);
    m_settings = new QSettings("skarsnik.nyo.fr", "SaveState2SNES");
    QString mode = m_settings->value("mode").toString();
    snesClassicInit = false;
    usb2snesInit = false;
    nwaccessInit = false;
    nwaclient = nullptr;
    usb2snes = nullptr;
    stuffControlCo = nullptr;
    stuffInput = nullptr;
    sDebug() << "Mode is " << mode;
    localController = nullptr;
    ui->nwaccessStatut->setSettings(m_settings);
    if (mode == "USB2Snes" || mode.isEmpty())
    {
        m_mode = USB2Snes;
        initUsb2snes();
        ui->tabWidget->setCurrentIndex(0);
        ui->usb2snesStackedWidget->setCurrentIndex(1);
        ui->nwaccessStackedWidget->setCurrentIndex(0);
        connect(ui->usb2snesStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()));
        connect(ui->usb2snesStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()));
    }
    if (mode == "SNESClassic")
    {
        m_mode = SNESClassic;
        initSnesClassic();
        ui->tabWidget->setCurrentIndex(1);
        ui->snesclassicStackedWidget->setCurrentIndex(1);
        ui->usb2snesStackedWidget->setCurrentIndex(0);
        ui->nwaccessStackedWidget->setCurrentIndex(0);
        connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()));
        connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SLOT(on_snesClassicReadyForSaveState()));
        connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()));
        connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SLOT(on_snesClassicUnReadyForSaveState()));
    }
    if (mode == "NWAccess")
    {
        m_mode = NWAccess;
        initNWAccess();
        ui->tabWidget->setCurrentIndex(2);
        ui->snesclassicStackedWidget->setCurrentIndex(0);
        ui->usb2snesStackedWidget->setCurrentIndex(0);
        ui->nwaccessStackedWidget->setCurrentIndex(1);
        connect(ui->nwaccessStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()));
        connect(ui->nwaccessStatut, SIGNAL(readyForSaveState()), this, SLOT(on_nwaReadyForSaveState()));
        connect(ui->nwaccessStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()));
        connect(ui->nwaccessStatut, SIGNAL(unReadyForSaveState()), this, SLOT(on_nwaUnReadyForSaveState()));
        QTimer::singleShot(0, this, [=]
        {
            ui->nwaccessStatut->setToShow();
            onLocalControllerChanged();
        });
    }
    snesClassicInputDecoder = new InputDecoder();
    snesClassicShortcutActivated = false;
    snesClassicReady = false;
    connect(snesClassicInputDecoder, &InputDecoder::buttonPressed, this, &ConsoleSwitcher::onInputProviderButtonPressed);
    connect(snesClassicInputDecoder, &InputDecoder::buttonReleased, this, &ConsoleSwitcher::onInputProviderButtonReleased);
    connect(ui->snesclassicStatut, SIGNAL(shortcutsToggled(bool)), this, SLOT(on_snesClassicShortcutsToggled(bool)));
    connect(ui->nwaccessStatut, &NWAccessStatut::localControllerChanged, this, &ConsoleSwitcher::onLocalControllerChanged);
    mapEnumToSNES[InputProvider::A] = B_A_BITMASK;
    mapEnumToSNES[InputProvider::B] = B_B_BITMASK;
    mapEnumToSNES[InputProvider::Y] = B_Y_BITMASK;
    mapEnumToSNES[InputProvider::X] = B_X_BITMASK;
    mapEnumToSNES[InputProvider::Start] = B_START_BITMASK;
    mapEnumToSNES[InputProvider::Select] = B_SELECT_BITMASK;
    mapEnumToSNES[InputProvider::L] = B_L_BITMASK;
    mapEnumToSNES[InputProvider::R] = B_R_BITMASK;
    mapEnumToSNES[InputProvider::Left] = B_LEFT_BITMASK;
    mapEnumToSNES[InputProvider::Right] = B_RIGHT_BITMASK;
    mapEnumToSNES[InputProvider::Up] = B_UP_BITMASK;
    mapEnumToSNES[InputProvider::Down] = B_DOWN_BITMASK;
}

HandleStuff *ConsoleSwitcher::getHandle()
{

    if (m_mode == USB2Snes)
        return handleUSB2Snes;
    if (m_mode == SNESClassic)
        return handleSNESClassic;
    return handleNWAccess;
}

void ConsoleSwitcher::start()
{
    if (m_mode == USB2Snes)
        usb2snes->connect();
    if (m_mode == SNESClassic)
    {
        stuffControlCo->connect();
        if (m_settings->value("SNESClassicShortcuts").toBool())
            stuffInput->connect();
    }
    if (m_mode == NWAccess)
    {
        nwaclient->connectToHost("127.0.0.1", 0xBEEF);
        connect(nwaclient, &EmuNWAccessClient::connected, this, [=] {
            nwaclient->cmdMyNameIs("Savestate2Snes control connection");
            connect(nwaclient, &EmuNWAccessClient::readyRead, this, [=]
            {
                nwaclient->readReply(); // This empty the reply queue
                disconnect(nwaclient, &EmuNWAccessClient::readyRead, this, nullptr);
                handleNWAccess->setNWAClient(nwaclient);
            });
        });
        ui->nwaccessStatut->start();
    }
}

ConsoleSwitcher::Mode ConsoleSwitcher::mode()
{
    return m_mode;
}

QString ConsoleSwitcher::readyString() const
{
    if (m_mode == USB2Snes)
        return ui->usb2snesStatut->readyString();
    if (m_mode == SNESClassic)
        return ui->snesclassicStatut->readyString();
    if (m_mode == NWAccess)
        return ui->nwaccessStatut->readyString();
}

QString ConsoleSwitcher::unreadyString() const
{
    if (m_mode == USB2Snes)
        return ui->usb2snesStatut->unreadyString();
    if (m_mode == SNESClassic)
        return ui->snesclassicStatut->unreadyString();
    if (m_mode == NWAccess)
        return ui->nwaccessStatut->unreadyString();
}

ConsoleSwitcher::~ConsoleSwitcher()
{
    qDebug() << "DELETE CONSOLESWITCHER";
    if (m_mode == USB2Snes)
    {
        m_settings->setValue("mode", "USB2Snes");
        cleanUpUSB2Snes();
        delete usb2snes;
    }
    if (m_mode == SNESClassic)
    {
        m_settings->setValue("mode", "SNESClassic");
        cleanUpSNESClassic();
    }
    if (m_mode == NWAccess)
    {
        m_settings->setValue("mode", "NWAccess");
        cleanUpNWAccess();
    }
    qDebug() << "DELETE UI";
    delete ui;
}

void ConsoleSwitcher::refreshShortcuts()
{
    if (m_mode == USB2Snes)
        ui->usb2snesStatut->refreshShortcuts();
}

void ConsoleSwitcher::onLocalControllerChanged()
{
    localController = ui->nwaccessStatut->localController;
    sDebug() << "Local Controller changed";
    connect(localController, &LocalController::buttonPressed, this, &ConsoleSwitcher::onInputProviderButtonPressed, Qt::UniqueConnection);
    connect(localController, &LocalController::buttonReleased, this, &ConsoleSwitcher::onInputProviderButtonReleased, Qt::UniqueConnection);
}

void ConsoleSwitcher::onInputProviderButtonPressed(InputProvider::SNESButton but)
{
    inputProviderButtonPressed.append(but);
    int currentInput = 0;
    foreach(int button, inputProviderButtonPressed)
    {
        currentInput = currentInput | mapEnumToSNES[button];
    }
    sDebug() << QString::number(currentInput, 16) << QString::number(getHandle()->shortcutLoad(), 16) << QString::number(getHandle()->shortcutSave(), 16);
    if (currentInput == getHandle()->shortcutLoad())
        getHandle()->controllerLoadState();
    if (currentInput == getHandle()->shortcutSave())
        getHandle()->controllerSaveState();
}

void ConsoleSwitcher::onInputProviderButtonReleased(InputProvider::SNESButton but)
{
    inputProviderButtonPressed.removeAll(but);
}

void ConsoleSwitcher::on_snesClassicInputNewData(QByteArray data)
{
    snesClassicInputDecoder->decodeBinary(data);
}

void ConsoleSwitcher::on_snesClassicInputConnected()
{
    sDebug() << "Input co connected";
    stuffInput->streamFile("/dev/input/by-path/platform-twi.1-event-joystick");
}

void ConsoleSwitcher::on_snesClassicShortcutsToggled(bool toggled)
{
    sDebug() << "Snes classic shortcut toggled : " << toggled;
    snesClassicShortcutActivated = toggled;
    if (toggled)
    {
        if (!stuffInput->isConnected())
            stuffInput->connect();

    } else {
        if (stuffInput != nullptr)
            stuffInput->close();
    }
    m_settings->setValue("SNESClassicShortcuts", toggled);
}

void ConsoleSwitcher::on_snesClassicReadyForSaveState()
{
    snesClassicReady = true;
    if (snesClassicShortcutActivated && !stuffInput->isConnected())
        stuffInput->connect();
}

void ConsoleSwitcher::on_snesClassicUnReadyForSaveState()
{
    snesClassicReady = false;
    if (snesClassicShortcutActivated)
        stuffInput->close();
}

void ConsoleSwitcher::initUsb2snes()
{
    usb2snes = new USB2snes("SaveState2Snes control");
    handleUSB2Snes = new HandleStuffUsb2snes();
    ui->usb2snesStatut->setUsb2snes();
    handleUSB2Snes->setUsb2snes(usb2snes);
    usb2snesInit = true;
}

void ConsoleSwitcher::initSnesClassic()
{
    sDebug() << "Init SNES Classic";
    stuffControlCo = new StuffClient(SNES_CLASSIC_IP);
    stuffInput = new StuffClient(SNES_CLASSIC_IP);
    ui->snesclassicStatut->setStuff(stuffControlCo);
    handleSNESClassic = new HandleStuffSnesClassic();
    handleSNESClassic->setControlCo(stuffControlCo);
    connect(stuffInput, &StuffClient::newFileData, this, &ConsoleSwitcher::on_snesClassicInputNewData);
    connect(stuffInput, &StuffClient::connected, this, &ConsoleSwitcher::on_snesClassicInputConnected);
    snesClassicInit = true;
}

void ConsoleSwitcher::initNWAccess()
{
    sDebug() << "Init Emu NWAccess";
    nwaclient = new EmuNWAccessClient();
    handleNWAccess = new HandleStuffNWAccess();
    //handleNWAccess->setNWAClient(nwaclient);
    nwaccessInit = true;
}

void ConsoleSwitcher::cleanUpUSB2Snes()
{
    if (usb2snes != nullptr)
    {
        usb2snes->close();
        ui->usb2snesStatut->stop();
    }
}

void ConsoleSwitcher::cleanUpSNESClassic()
{
    if (stuffControlCo != nullptr)
        stuffControlCo->close();
    ui->snesclassicStatut->stop();
}

void ConsoleSwitcher::cleanUpNWAccess()
{
    nwaclient->disconnectFromHost();
}

void ConsoleSwitcher::on_snesClassicButton_clicked()
{
    emit unReadyForSaveState();
    cleanUpUSB2Snes();
    cleanUpNWAccess();
    if (!snesClassicInit)
        initSnesClassic();
    m_mode = SNESClassic;
    start();
    ui->snesclassicStackedWidget->setCurrentIndex(1);
    ui->usb2snesStackedWidget->setCurrentIndex(0);
    ui->nwaccessStackedWidget->setCurrentIndex(0);
    disconnect(ui->usb2snesStatut, nullptr, this, nullptr);
    disconnect(ui->nwaccessStatut, nullptr, this, nullptr);
    connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()), Qt::UniqueConnection);
    connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SLOT(on_snesClassicReadyForSaveState()), Qt::UniqueConnection);
    connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()), Qt::UniqueConnection);
    connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SLOT(on_snesClassicUnReadyForSaveState()), Qt::UniqueConnection);
    emit modeChanged(SNESClassic);
}

void ConsoleSwitcher::on_nwaReadyForSaveState()
{
    if (localController != nullptr)
    {
        localController->start();
    }
    if (!nwaclient->isConnected())
        nwaclient->connectToHost("127.0.0.1", 0xBEEF);
}

void ConsoleSwitcher::on_nwaUnReadyForSaveState()
{
    if (localController != nullptr)
    {
        localController->stop();
    }
    nwaclient->disconnectFromHost();
}

void ConsoleSwitcher::on_usb2snesButton_clicked()
{
    emit unReadyForSaveState();
    cleanUpSNESClassic();
    cleanUpNWAccess();
    if (!usb2snesInit)
        initUsb2snes();
    m_mode = USB2Snes;
    start();
    ui->snesclassicStackedWidget->setCurrentIndex(0);
    ui->usb2snesStackedWidget->setCurrentIndex(1);
    ui->nwaccessStackedWidget->setCurrentIndex(0);
    disconnect(ui->snesclassicStatut, nullptr, this, nullptr);
    disconnect(ui->nwaccessStatut, nullptr, this, nullptr);
    connect(ui->usb2snesStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()), Qt::UniqueConnection);
    connect(ui->usb2snesStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()), Qt::UniqueConnection);

    emit modeChanged(USB2Snes);
}

void ConsoleSwitcher::on_emunwaccessButton_clicked()
{
    emit unReadyForSaveState();
    cleanUpUSB2Snes();
    cleanUpSNESClassic();
    if (!nwaccessInit)
        initNWAccess();
    m_mode = NWAccess;
    ui->nwaccessStackedWidget->setCurrentIndex(1);
    ui->snesclassicStackedWidget->setCurrentIndex(0);
    ui->usb2snesStackedWidget->setCurrentIndex(0);
    start();
    disconnect(ui->snesclassicStatut, nullptr, this, nullptr);
    disconnect(ui->usb2snesStatut, nullptr, this, nullptr);
    connect(ui->nwaccessStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()), Qt::UniqueConnection);
    connect(ui->nwaccessStatut, SIGNAL(readyForSaveState()), this, SLOT(on_nwaReadyForSaveState()), Qt::UniqueConnection);
    connect(ui->nwaccessStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()), Qt::UniqueConnection);
    connect(ui->nwaccessStatut, SIGNAL(unReadyForSaveState()), this, SLOT(on_nwaUnReadyForSaveState()), Qt::UniqueConnection);
    emit modeChanged(NWAccess);
}
