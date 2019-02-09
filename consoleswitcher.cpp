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

ConsoleSwitcher::ConsoleSwitcher(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConsoleSwitcher)
{
    ui->setupUi(this);
    m_settings = new QSettings("skarsnik.nyo.fr", "SaveState2SNES");
    QString mode = m_settings->value("mode").toString();
    snesClassicInit = false;
    usb2snesInit = false;
    stuffControlCo = nullptr;
    stuffInput = nullptr;
    sDebug() << "Mode is " << mode;
    if (mode == "USB2Snes" || mode.isEmpty())
    {
        m_mode = USB2Snes;
        initUsb2snes();
        ui->tabWidget->setCurrentIndex(0);
        ui->usb2snesStackedWidget->setCurrentIndex(1);
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
        connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()));
        connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SLOT(on_snesClassicReadyForSaveState()));
        connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()));
        connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SLOT(on_snesClassicUnReadyForSaveState()));
    }
    snesClassicInputDecoder = new InputDecoder();
    snesClassicShortcutActivated = false;
    snesClassicReady = false;
    connect(snesClassicInputDecoder, SIGNAL(buttonPressed(InputDecoder::SNESButton)), this, SLOT(on_snesClassicInputDecoderButtonPressed(InputDecoder::SNESButton)));
    connect(snesClassicInputDecoder, SIGNAL(buttonReleased(InputDecoder::SNESButton)), this, SLOT(on_snesClassicInputDecoderButtonReleased(InputDecoder::SNESButton)));
    connect(ui->snesclassicStatut, SIGNAL(shortcutsToggled(bool)), this, SLOT(on_snesClassicShortcutsToggled(bool)));
    mapEnumToSNES[InputDecoder::A] = B_A_BITMASK;
    mapEnumToSNES[InputDecoder::B] = B_B_BITMASK;
    mapEnumToSNES[InputDecoder::Y] = B_Y_BITMASK;
    mapEnumToSNES[InputDecoder::X] = B_Y_BITMASK;
    mapEnumToSNES[InputDecoder::Start] = B_START_BITMASK;
    mapEnumToSNES[InputDecoder::Select] = B_SELECT_BITMASK;
    mapEnumToSNES[InputDecoder::L] = B_L_BITMASK;
    mapEnumToSNES[InputDecoder::R] = B_R_BITMASK;
    mapEnumToSNES[InputDecoder::Left] = B_LEFT_BITMASK;
    mapEnumToSNES[InputDecoder::Right] = B_RIGHT_BITMASK;
    mapEnumToSNES[InputDecoder::Up] = B_UP_BITMASK;
    mapEnumToSNES[InputDecoder::Down] = B_DOWN_BITMASK;
}

HandleStuff *ConsoleSwitcher::getHandle()
{
    if (m_mode == USB2Snes)
        return handleUSB2Snes;
    return handleSNESClassic;
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
}

QString ConsoleSwitcher::unreadyString() const
{
    if (m_mode == USB2Snes)
        return ui->usb2snesStatut->unreadyString();
    if (m_mode == SNESClassic)
        return ui->snesclassicStatut->unreadyString();
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
    qDebug() << "DELETE UI";
    delete ui;
}

void ConsoleSwitcher::refreshShortcuts()
{
    if (m_mode == USB2Snes)
        ui->usb2snesStatut->refreshShortcuts();
}

void ConsoleSwitcher::on_snesClassicInputDecoderButtonPressed(InputDecoder::SNESButton but)
{
    snesClassicButtonPressed.append(but);
    sDebug() << snesClassicButtonPressed.size();
    int currentInput = 0;
    foreach(int button, snesClassicButtonPressed)
    {
        currentInput = currentInput | mapEnumToSNES[button];
    }
    sDebug() << QString::number(currentInput, 16) << QString::number(handleSNESClassic->shortcutLoad(), 16) << QString::number(handleSNESClassic->shortcutSave(), 16);
    if (currentInput == handleSNESClassic->shortcutLoad())
        handleSNESClassic->controllerLoadState();
    if (currentInput == handleSNESClassic->shortcutSave())
        handleSNESClassic->controllerSaveState();
}

void ConsoleSwitcher::on_snesClassicInputDecoderButtonReleased(InputDecoder::SNESButton but)
{
    snesClassicButtonPressed.removeAll(but);
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
    usb2snes = new USB2snes();
    handleUSB2Snes = new HandleStuffUsb2snes();
    ui->usb2snesStatut->setUsb2snes(usb2snes);
    handleUSB2Snes->setUsb2snes(usb2snes);
    usb2snesInit = true;
}

void ConsoleSwitcher::initSnesClassic()
{
    sDebug() << "Init SNES Classic";
    stuffControlCo = new StuffClient();
    stuffInput = new StuffClient();
    ui->snesclassicStatut->setStuff(stuffControlCo);
    handleSNESClassic = new HandleStuffSnesClassic();
    handleSNESClassic->setControlCo(stuffControlCo);
    connect(stuffInput, &StuffClient::newFileData, this, &ConsoleSwitcher::on_snesClassicInputNewData);
    connect(stuffInput, &StuffClient::connected, this, &ConsoleSwitcher::on_snesClassicInputConnected);
    snesClassicInit = true;
}

void ConsoleSwitcher::cleanUpUSB2Snes()
{
    usb2snes->close();
    ui->usb2snesStatut->stop();
}

void ConsoleSwitcher::cleanUpSNESClassic()
{
    stuffControlCo->close();
    ui->snesclassicStatut->stop();
}

void ConsoleSwitcher::on_snesClassicButton_clicked()
{
    emit unReadyForSaveState();
    cleanUpUSB2Snes();
    if (!snesClassicInit)
        initSnesClassic();
    m_mode = SNESClassic;
    start();
    ui->snesclassicStackedWidget->setCurrentIndex(1);
    ui->usb2snesStackedWidget->setCurrentIndex(0);
    disconnect(ui->usb2snesStatut, 0, this, 0);
    connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()), Qt::UniqueConnection);
    connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SLOT(on_snesClassicReadyForSaveState()), Qt::UniqueConnection);
    connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()), Qt::UniqueConnection);
    connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SLOT(on_snesClassicUnReadyForSaveState()), Qt::UniqueConnection);
    emit modeChanged(SNESClassic);
}

void ConsoleSwitcher::on_usb2snesButton_clicked()
{
    emit unReadyForSaveState();
    cleanUpSNESClassic();
    if (!usb2snesInit)
        initUsb2snes();
    m_mode = USB2Snes;
    start();
    ui->snesclassicStackedWidget->setCurrentIndex(0);
    ui->usb2snesStackedWidget->setCurrentIndex(1);
    disconnect(ui->snesclassicStatut, 0, this, 0);
    connect(ui->usb2snesStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()), Qt::UniqueConnection);
    connect(ui->usb2snesStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()), Qt::UniqueConnection);

    emit modeChanged(USB2Snes);
}
