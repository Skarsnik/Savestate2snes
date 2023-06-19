#include "trainingconfigdialog.h"
#include "ui_trainingconfigdialog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

TrainingConfigDialog::TrainingConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrainingConfigDialog)
{
    ui->setupUi(this);

}

TrainingConfigDialog::~TrainingConfigDialog()
{
    delete ui;
}

bool TrainingConfigDialog::loadPresets(const QString& path)
{
    ui->platformComboBox->clear();
    QFile jsonFile(path);
    qDebug() << "Processing memorypreset file";
    jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray jsonStr = jsonFile.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr);
    if (jsonDoc.isNull())
        return false;
    QJsonObject jsonObj = jsonDoc.object();
    for (auto key : jsonObj.keys())
    {
        qDebug() << "New plateform :" << key;
        m_plateforms.append(key);
        ui->platformComboBox->addItem(key);
        QJsonArray gameList = jsonObj.value(key).toArray();
        for (auto gameRef : gameList)
        {
            bool ok;
            QJsonObject gameInfos = gameRef.toObject();
            MemoryPreset newPreset;
            newPreset.gameName = gameInfos.value("name").toString();
            newPreset.domain = gameInfos.value("domain").toString();
            newPreset.address = gameInfos.value("address").toString().toUInt(&ok, 16);
            newPreset.size = gameInfos.value("size").toInt();
            qDebug() << "New preset : " << newPreset.gameName << newPreset.domain << newPreset.address << newPreset.size;
            presets[key].append(newPreset);
        }
        if (m_plateforms.size() == 1)
            on_platformComboBox_currentTextChanged(key);
    }
    return true;
}

MemoryPreset TrainingConfigDialog::currentPreset() const
{
    return m_currentPreset;
}

void TrainingConfigDialog::setPreset(const MemoryPreset preset)
{
    m_currentPreset = preset;
    if (preset.size != 0)
        setLineEdit(m_currentPreset);
}

void TrainingConfigDialog::on_platformComboBox_currentTextChanged(const QString &plateform)
{
    ui->gameComboBox->clear();
    for (auto preset : presets[plateform])
    {
        ui->gameComboBox->addItem(preset.gameName);
    }
}

void    TrainingConfigDialog::setLineEdit(const MemoryPreset preset)
{
    ui->lineEdit->setText(QString("%1:%2:%3").arg(m_currentPreset.domain).arg(m_currentPreset.address, 0, 16).arg(m_currentPreset.size));
}

void TrainingConfigDialog::on_gameComboBox_currentIndexChanged(int index)
{
    m_currentPreset = presets[ui->platformComboBox->currentText()][index];
    setLineEdit(m_currentPreset);
}


void TrainingConfigDialog::on_lineEdit_textEdited(const QString &text)
{
    if (parseLineEdit(text))
        ui->validLabel->setText(tr("Valid"));
}

bool TrainingConfigDialog::parseLineEdit(const QString& text)
{
    QStringList list = text.split(':');
    if (list.size() != 3)
    {
        ui->validLabel->setText(tr("Invalid format, missing some data"));
        return false;
    }
    bool    ok;
    QString domain = list.at(0);
    unsigned int address = list.at(1).toInt(&ok, 16);
    if (!ok)
    {
        ui->validLabel->setText(tr("Invalid format for address"));
        return false;
    }
    unsigned int size = list.at(2).toInt(&ok);
    if (!ok)
    {
        ui->validLabel->setText(tr("Invalid format for size"));
        return false;
    }
    m_currentPreset.gameName = "Manual";
    m_currentPreset.domain = domain;
    m_currentPreset.address = address;
    m_currentPreset.size = size;
    return true;
}




void TrainingConfigDialog::on_buttonBox_accepted()
{
    if (parseLineEdit(ui->lineEdit->text()))
        accept();
    else
        reject();
}

