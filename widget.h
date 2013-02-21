#ifndef WIDGET_H
#define WIDGET_H

#include <QtWidgets/QWidget>
#include <QString>
#include <QHash>
#include <QtWidgets/QFileDialog>
#include <QDebug>
#include <QElapsedTimer>
#include <QtWidgets/QMessageBox>
#include <QStandardItemModel>



namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    
private:
    Ui::Widget *ui;    
    QStandardItemModel *model;

    bool isWord;
    unsigned int totalWords, uniqueWords, textChar, sepChar, counter, wordBegin;
    QString separators, text;
    QHash<QString, int> words;

    bool *sp;

    void processChar(const QChar &ch);
    void writeText(unsigned long long t);

    void indexChars();
    void updateModel();

private slots:
    void startAnalysis();
    void updateSeparators();

    void fileToString();

    void editText();
    void doneText();
    void resetSeparators();
    void saveSep();
    void loadSep();
    void benchmark();
};

#endif // WIDGET_H
