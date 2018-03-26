#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(log_ShortCutDialog, "ShortcutDialog")

#define sDebug() qCDebug(log_ShortCutDialog)

#include "shortcuteditdialog.h"
#include "ui_shortcuteditdialog.h"

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


QMap<quint16, QString> maskToButton;


ShortcutEditDialog::ShortcutEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutEditDialog)
{

}

ShortcutEditDialog::ShortcutEditDialog(QWidget *parent, quint16 save, quint16 load) :
    QDialog(parent),
    ui(new Ui::ShortcutEditDialog)
{
    sDebug() << "Shortcutedit created : s/l" << QString::number(save, 16) << QString::number(load, 16);
    m_saveShortcut = save;
    m_loadShortcut = load;
    maskToButton[B_START_BITMASK] = "start";
    maskToButton[B_SELECT_BITMASK] = "select";
    maskToButton[B_A_BITMASK] = "A";
    maskToButton[B_B_BITMASK] = "B";
    maskToButton[B_Y_BITMASK] = "Y";
    maskToButton[B_X_BITMASK] = "X";
    maskToButton[B_L_BITMASK] = "L";
    maskToButton[B_R_BITMASK] = "R";
    maskToButton[B_RIGHT_BITMASK] = "right";
    maskToButton[B_LEFT_BITMASK] = "left";
    maskToButton[B_DOWN_BITMASK] = "down";
    maskToButton[B_UP_BITMASK] = "up";
    ui->setupUi(this);

    ui->controllerButtonGroup->setId(ui->APushButton, B_A_BITMASK);
    ui->controllerButtonGroup->setId(ui->BPushButton, B_B_BITMASK);
    ui->controllerButtonGroup->setId(ui->YPushButton, B_Y_BITMASK);
    ui->controllerButtonGroup->setId(ui->XPushButton, B_X_BITMASK);
    ui->controllerButtonGroup->setId(ui->selectPushButton, B_SELECT_BITMASK);
    ui->controllerButtonGroup->setId(ui->startPushButton, B_START_BITMASK);
    ui->controllerButtonGroup->setId(ui->LPushButton, B_L_BITMASK);
    ui->controllerButtonGroup->setId(ui->RPushButton, B_R_BITMASK);
    ui->controllerButtonGroup->setId(ui->upPushButton, B_UP_BITMASK);
    ui->controllerButtonGroup->setId(ui->downPushButton, B_DOWN_BITMASK);
    ui->controllerButtonGroup->setId(ui->leftPushButton, B_LEFT_BITMASK);
    ui->controllerButtonGroup->setId(ui->rightPushButton, B_RIGHT_BITMASK);

    connect(ui->controllerButtonGroup, SIGNAL(buttonToggled(int,bool)), this, SLOT(onControllerGroupToggled(int,bool)));
    connect(ui->chooseButtonGroup, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(onChooseGroupToggled(QAbstractButton*, bool)));

    setControllerButtonStatus(m_saveShortcut);
    setLabels();
}

ShortcutEditDialog::~ShortcutEditDialog()
{
    delete ui;
}

quint16 ShortcutEditDialog::saveShortcut()
{
    return m_saveShortcut;
}

quint16 ShortcutEditDialog::loadShortcut()
{
    return m_loadShortcut;
}

void    ShortcutEditDialog::setLabels()
{
    quint16 shortcut = m_saveShortcut;
    QStringList inputs;
    foreach(quint16 mask, maskToButton.keys())
    {
        if ((mask & shortcut) == mask)
            inputs << maskToButton[mask];
    }
    ui->saveLineEdit->setText(inputs.join("+"));
    shortcut = m_loadShortcut;
    inputs.clear();
    foreach(quint16 mask, maskToButton.keys())
    {
        if ((mask & shortcut) == mask)
            inputs << maskToButton[mask];
    }
    ui->loadLineEdit->setText(inputs.join("+"));
}

void ShortcutEditDialog::onControllerGroupToggled(int id, bool checked)
{
    QStringList inputs;

    qDebug() << maskToButton[(quint16)id] << checked;
    quint16&    shortcut = ui->loadRadioButton->isChecked() ? m_loadShortcut : m_saveShortcut;
    if (checked)
        shortcut = shortcut | ((quint16) id);
    else
        shortcut = (shortcut ^ (quint16) id) & shortcut;
    foreach(quint16 mask, maskToButton.keys())
    {
        if ((mask & shortcut) == mask)
            inputs << maskToButton[mask];
    }
    if (ui->loadRadioButton->isChecked())
        ui->loadLineEdit->setText(inputs.join("+"));
    else
        ui->saveLineEdit->setText(inputs.join("+"));
    sDebug() << "Load" << QString::number(m_loadShortcut, 16);
    sDebug() << "Save" << QString::number(m_saveShortcut, 16);
}

void ShortcutEditDialog::onChooseGroupToggled(QAbstractButton *button, bool toggled)
{
    if (button == ui->saveRadioButton && toggled)
    {
        setControllerButtonStatus(m_saveShortcut);
        ui->whoEditLabel->setText(tr("Editing shortcut for saving"));
    }
    if (button == ui->loadRadioButton && toggled)
    {
        setControllerButtonStatus(m_loadShortcut);
        ui->whoEditLabel->setText(tr("Editing shortcut for loading"));
    }
}

void ShortcutEditDialog::setControllerButtonStatus(quint16 shortcut)
{
    foreach(QAbstractButton* but, ui->controllerButtonGroup->buttons())
    {
        but->setChecked(false);
    }
    foreach(quint16 mask, maskToButton.keys())
    {
        if ((mask & shortcut) == mask)
            ui->controllerButtonGroup->button(mask)->setChecked(true);
    }
}

