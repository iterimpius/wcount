#include "widget.h"
#include "ui_widget.h"
#include <QPainter>


Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    sp = new bool[65536];
    isWord = totalWords = uniqueWords = textChar = sepChar = counter = wordBegin = 0;
    model = 0;

    resetSeparators();
    updateSeparators();


    ui->tableView->setSortingEnabled(1);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->beginBtn->setEnabled(0);
    ui->status->setAlignment(Qt::AlignCenter);
    ui->status->setText("IDLE");


    connect(ui->selectBtn, SIGNAL(clicked()), this, SLOT(fileToString()));
    connect(ui->beginBtn, SIGNAL(clicked()), this, SLOT(startAnalysis()));
    connect(ui->editBtn, SIGNAL(clicked()), this, SLOT(editText()));
    connect(ui->doneBtn, SIGNAL(clicked()), this, SLOT(doneText()));
    connect(ui->loadSepBtn, SIGNAL(clicked()), this, SLOT(loadSep()));
    connect(ui->saveSepBtn, SIGNAL(clicked()), this, SLOT(saveSep()));
    connect(ui->defaultSepBtn, SIGNAL(clicked()), this, SLOT(resetSeparators()));
    connect(ui->benchBtn, SIGNAL(clicked()), this, SLOT(benchmark()));
    connect(ui->separatorsEdit, SIGNAL(textChanged(QString)), this, SLOT(updateSeparators()));

    ui->tabWidget->setCurrentIndex(0);
}

Widget::~Widget()
{
    delete ui;
    delete [] sp;
}

void Widget::processChar(const QChar &ch)
{
    // if char is not separator
    if (!sp[ch.unicode()]){
        if (!isWord){ // set where word begins
            wordBegin = counter;
            isWord = 1;
        }
        ++textChar;
    }
    // if char is separator
    else{
        // if there is a current word
        if (isWord){
            QString tempWord(text.mid(wordBegin, counter - wordBegin));
            tempWord = tempWord.toLower();
            // if word does not exist add with value one
            if (!words.contains(tempWord)){
                words.insert(tempWord, 1);
                ++uniqueWords;
            }
            // if word exists find its value and increment by one
            else {
                words.insert(tempWord, words.value(tempWord) + 1);
            }
            ++totalWords;            
            isWord = 0;
        }
        // else skip
        ++sepChar;
    }
}

void Widget::writeText(unsigned long long t){
    ui->uniqueWordsLbl->setText(QString::number(uniqueWords));
    ui->totalWordsLbl->setText(QString::number(totalWords));
    ui->textCharLbl->setText(QString::number(textChar));
    ui->sepCharLbl->setText(QString::number(sepChar-1));
    ui->totalCharLbl->setText(QString::number(text.length()));
    ui->timeLbl->setText(QString::number((double)(t / 1000000)));
    ui->mcpiLbl->setText(QString::number((double)(text.length())/((double)t/1000.0)));
}


void Widget::resetSeparators()
{
    // append some separators
    separators.clear();
    separators.append(QStringLiteral(" .,\n()<>!?;:|\"\'[]+-/\\\t\v\r#@*={}"));
    separators.append(QChar(8220));
    separators.append(QChar(8221));
    separators.append(QChar(8230));
    separators.append(QChar(8211));
    separators.append(QChar(8212));
    separators.append(QChar(8216));
    separators.append(QChar(8217));
    separators.append(QChar(8218));
    ui->separatorsEdit->setText(separators);
}

void Widget::saveSep()
{
    // get file name
    QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("Save Text File"), "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;

    // open a file
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        return;
    }

    // copy file into the string
    QTextStream out(&file);
    out << separators;

    // flush
    file.close();
}

void Widget::loadSep()
{
    // get file name
    QString fileName = QFileDialog::getOpenFileName(this,QStringLiteral("Open Text File"), "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;

    separators.clear();

    // open a file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }

    // copy file into the string
    QTextStream in(&file);
    separators = in.readAll();

    ui->separatorsEdit->setText(separators);

    // flush
    file.close();

    indexChars();
}

void Widget::benchmark()
{
    ui->beginBtn->setEnabled(0);
    resetSeparators();
    srand(666);

    text.clear();
    text.reserve(7003000);

    for (int i = 0; i < 2000000; ++i) {
        unsigned length = (qrand() % 2) + 2;
        for (unsigned c = 0; c < length; ++c) {
            text.append(QChar((qrand() % 15) + 105));
        }
        text.append(" ");
    }

    startAnalysis();

    ui->beginBtn->setEnabled(0);
}

void Widget::indexChars()
{
    // zero everything
    for (unsigned i = 0; i < 65536; ++i) sp[i] = 0;

    // set every separator index to 1
    for (int i = 0; i < separators.length(); ++i) {
        sp[separators.at(i).unicode()] = 1;
    }
}

void Widget::updateModel()
{
    if (model) {
        model->clear();
        delete model;
    }
    model = new QStandardItemModel(words.size(), 2, this);
    QHash<QString, int>::iterator iter;
    int count = 0;
    for (iter = words.begin(); iter != words.end(); ++iter){
        QModelIndex index1 = model->index(count, 0);
        QModelIndex index2 = model->index(count, 1);
        model->setData(index1, iter.key());
        model->setData(index2, iter.value());
        count++;
    }
    ui->tableView->setModel(model);

}

void Widget::fileToString()
{
    ui->beginBtn->setEnabled(0);
    ui->plainTextEdit->setEnabled(0);
    ui->editBtn->setEnabled(1);
    ui->doneBtn->setEnabled(0);

    // get file name
    QString fileName = QFileDialog::getOpenFileName(this, "Open Text File", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()){
        ui->beginBtn->setEnabled(1);
        this->setWindowTitle(QStringLiteral("Analyzing ") + fileName);
    }

    // clear string
    text.clear();

    // open a file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;


    // copy file into the string
    QTextStream in(&file);
    text.reserve(file.size());
    text = in.readAll();

    // flush
    file.close();

    ui->plainTextEdit->clear();
    if (text.length() < 1000000) ui->plainTextEdit->setPlainText(text);
    else ui->plainTextEdit->setPlainText(QStringLiteral("Text is too large to display"));

    ui->tabWidget->setCurrentIndex(0);
}

void Widget::editText()
{
    ui->beginBtn->setEnabled(0);
    ui->editBtn->setEnabled(0);
    ui->doneBtn->setEnabled(1);
    ui->plainTextEdit->setEnabled(1);
}

void Widget::doneText()
{
    ui->beginBtn->setEnabled(1);
    ui->editBtn->setEnabled(1);
    ui->doneBtn->setEnabled(0);
    ui->plainTextEdit->setEnabled(0);
    text = ui->plainTextEdit->toPlainText();
}


void Widget::startAnalysis()
{
    ui->status->setText("WORKING");
    // check if there are no separators
    if (separators.isEmpty()){
        QMessageBox msg;
        msg.setText(QStringLiteral("Error: There are no separators"));
        msg.exec();
        return;
    }
    ui->beginBtn->setEnabled(0);
    ui->selectBtn->setEnabled(0);
    ui->separatorsEdit->setEnabled(0);
    ui->benchBtn->setEnabled(0);

    // cleanup
    totalWords = uniqueWords = textChar = sepChar = counter = wordBegin = 0;
    words.clear();
    //word.clear();

    unsigned c = text.length();
    ui->prog->setMaximum(c);

    // time
    QElapsedTimer timer;
    timer.start();

    // work loop
    for (; counter < c; ++counter){
        processChar(text.at(counter));
        if (!(counter % 2000)) ui->prog->setValue(counter);
    }

    // force-flush the last char
    processChar(separators[0]);
    ui->prog->setValue(c);

    writeText(timer.nsecsElapsed());
    updateModel();
    ui->tabWidget->setCurrentIndex(1);

    ui->status->setText("DONE");
    ui->benchBtn->setEnabled(1);
    ui->beginBtn->setEnabled(1);
    ui->selectBtn->setEnabled(1);
    ui->separatorsEdit->setEnabled(1);
}

void Widget::updateSeparators()
{
    separators = ui->separatorsEdit->text();
    if (!separators.isEmpty()) {
        indexChars();
    }
    else {
        ui->beginBtn->setEnabled(0);
    }
}
