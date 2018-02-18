#include "consoleswitcher.h"
#include "ui_consoleswitcher.h"

ConsoleSwitcher::ConsoleSwitcher(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConsoleSwitcher)
{
    ui->setupUi(this);
    m_settings = new QSettings("skarsnik.nyo.fr", "SaveState2SNES");
    QString mode = m_settings->value("mode").toString();
    snesClassicInit = false;
    usb2snesInit = false;
    if (mode == "USB2Snes" || mode.isEmpty())
    {
        m_mode = USB2Snes;
        initUsb2snes();
        ui->tabWidget->setCurrentIndex(0);
        ui->usb2snesStackedWidget->setCurrentIndex(1);
    }
    if (mode == "SNESClassic")
    {
        m_mode = SNESClassic;
        initSnesClassic();
        ui->tabWidget->setCurrentIndex(1);
        ui->snesclassicStackedWidget->setCurrentIndex(1);
    }
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
        telnetCommandCo->conneect();
        telnetCanoeCo->conneect();
        miniFTP->connect();
    }
}

ConsoleSwitcher::Mode ConsoleSwitcher::mode()
{
    return m_mode;
}

ConsoleSwitcher::~ConsoleSwitcher()
{
    /*if (m_mode == USB2Snes)
        m_settings->setValue("mode", "USB2Snes");
    if (m_mode == SNESClassic)
        m_settings->setValue("mode", "SNESClassic");*/
    delete ui;
}

void ConsoleSwitcher::refreshShortcuts()
{
    if (m_mode == USB2Snes)
        ui->usb2snesStatut->refreshShortcuts();
}

void ConsoleSwitcher::initUsb2snes()
{
    usb2snes = new USB2snes();
    handleUSB2Snes = new HandleStuffUsb2snes();
    ui->usb2snesStatut->setUsb2snes(usb2snes);
    handleUSB2Snes->setUsb2snes(usb2snes);
    connect(ui->usb2snesStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()));
    connect(ui->usb2snesStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()));
    usb2snesInit = true;
}

void ConsoleSwitcher::initSnesClassic()
{
    telnetCommandCo = new TelnetConnection("127.0.0.1", 1023, "root", "clover");
    telnetCommandCo->debugName = "Command";
    telnetCanoeCo = new TelnetConnection("127.0.0.1", 1023, "root", "clover");
    telnetCanoeCo->debugName = "Canoe";
    telnetInputCo = new TelnetConnection("127.0.0.1", 1023, "root", "clover");
    telnetInputCo->debugName = "Input";
    miniFTP = new MiniFtp(this);
    ui->snesclassicStatut->setCommandCo(telnetCommandCo, telnetCanoeCo);
    ui->snesclassicStatut->setFtp(miniFTP);
    connect(ui->snesclassicStatut, SIGNAL(readyForSaveState()), this, SIGNAL(readyForSaveState()));
    connect(ui->snesclassicStatut, SIGNAL(unReadyForSaveState()), this, SIGNAL(unReadyForSaveState()));
    handleSNESClassic = new HandleStuffSnesClassic();
    handleSNESClassic->setCommandCo(telnetCommandCo, telnetCanoeCo);
    snesClassicInit = true;
}

void ConsoleSwitcher::cleanUpUSB2Snes()
{
    usb2snes->close();
}

void ConsoleSwitcher::on_snesClassicButton_clicked()
{
    cleanUpUSB2Snes();
    if (!snesClassicInit)
        initSnesClassic();
    m_mode = SNESClassic;
    start();
    ui->snesclassicStackedWidget->setCurrentIndex(1);
    ui->usb2snesStackedWidget->setCurrentIndex(0);
    emit modeChanged(SNESClassic);
}
