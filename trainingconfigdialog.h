#ifndef TRAININGCONFIGDIALOG_H
#define TRAININGCONFIGDIALOG_H

#include <QDialog>
#include <QPair>
#include <QMap>
#include <QList>
#include <handlestuff.h>

namespace Ui {
class TrainingConfigDialog;
}

class TrainingConfigDialog : public QDialog
{
    Q_OBJECT

public:

    explicit TrainingConfigDialog(QWidget *parent = nullptr);
    ~TrainingConfigDialog();
    bool    loadPresets(const QString& path);
    QMap<QString, QList<MemoryPreset>>  presets;
    QStringList plateforms();
    MemoryPreset    currentPreset() const;
    void            setPreset(const MemoryPreset preset);

private slots:
    void on_platformComboBox_currentTextChanged(const QString &arg1);

    void on_gameComboBox_currentIndexChanged(int index);

    void on_lineEdit_textEdited(const QString &arg1);


    void on_buttonBox_accepted();

private:
    Ui::TrainingConfigDialog *ui;
    QStringList m_plateforms;
    MemoryPreset m_currentPreset;

    bool        parseLineEdit(const QString& text);

    void setLineEdit(const MemoryPreset preset);
};

#endif // TRAININGCONFIGDIALOG_H
