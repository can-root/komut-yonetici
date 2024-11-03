#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QInputDialog>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QTextEdit>
#include <QMessageBox>

class Pencere : public QWidget {
public:
    Pencere(QWidget *parent = nullptr) : QWidget(parent) {
        QVBoxLayout *anaLayout = new QVBoxLayout(this);

        listeWidget = new QListWidget(this);
        anaLayout->addWidget(listeWidget);

        QHBoxLayout *butonLayout = new QHBoxLayout();
        QPushButton *ekleButonu = new QPushButton("Ekle", this);
        QPushButton *silButonu = new QPushButton("Sil", this);
        QPushButton *duzenleButonu = new QPushButton("Düzenle", this);
        QPushButton *calistirButonu = new QPushButton("Çalıştır", this);

        butonLayout->addWidget(ekleButonu);
        butonLayout->addWidget(silButonu);
        butonLayout->addWidget(duzenleButonu);
        butonLayout->addWidget(calistirButonu);
        anaLayout->addLayout(butonLayout);

        outputTextEdit = new QTextEdit(this);
        outputTextEdit->setReadOnly(true);
        anaLayout->addWidget(outputTextEdit);

        connect(ekleButonu, &QPushButton::clicked, this, &Pencere::komutEkle);
        connect(silButonu, &QPushButton::clicked, this, &Pencere::komutSil);
        connect(duzenleButonu, &QPushButton::clicked, this, &Pencere::komutDuzenle);
        connect(calistirButonu, &QPushButton::clicked, this, &Pencere::komutCalistir);

        setLayout(anaLayout);
        setWindowTitle("Komut Yöneticisi");

        komutlariYukle();
        stilleriYukle();
    }

private:
    QListWidget *listeWidget;
    QTextEdit *outputTextEdit;
    const QString komutKlasoru = "komutlar/";

    void komutlariYukle() {
        QDir klasor(komutKlasoru);
        if (!klasor.exists()) {
            klasor.mkpath(".");
        }

        QStringList dosyalar = klasor.entryList(QDir::Files);
        for (const QString &dosya : dosyalar) {
            QFile komutDosyasi(komutKlasoru + dosya);
            if (komutDosyasi.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&komutDosyasi);
                QString komut = in.readLine();
                listeWidget->addItem(dosya + " : " + komut);
                komutDosyasi.close();
            }
        }
    }

    void stilleriYukle() {
        QFile dosya("styles.css");
        if (dosya.open(QFile::ReadOnly)) {
            QString stilSayfasi = dosya.readAll();
            setStyleSheet(stilSayfasi);
            dosya.close();
        }
    }

    void komutEkle() {
        bool ok;
        QString baslik = QInputDialog::getText(this, "Ekle", "Komut Başlığı:", QLineEdit::Normal, "", &ok);
        if (ok && !baslik.isEmpty()) {
            QString icerik = QInputDialog::getText(this, "Ekle", "Komut İçeriği:", QLineEdit::Normal, "", &ok);
            if (ok && !icerik.isEmpty()) {
                QString dosyaYolu = komutKlasoru + baslik + ".txt";
                QFile komutDosyasi(dosyaYolu);
                if (komutDosyasi.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&komutDosyasi);
                    out << icerik;
                    komutDosyasi.close();
                }
                listeWidget->addItem(baslik + ".txt : " + icerik);
            }
        }
    }

    void komutSil() {
        QListWidgetItem *currentItem = listeWidget->currentItem();
        if (currentItem) {
            QString currentText = currentItem->text();
            QStringList parcalar = currentText.split(" : ");
            if (parcalar.size() == 2) {
                QString baslik = parcalar[0];
                QFile::remove(komutKlasoru + baslik);
                delete currentItem;
            }
        }
    }

    void komutDuzenle() {
        QListWidgetItem *currentItem = listeWidget->currentItem();
        if (currentItem) {
            QString currentText = currentItem->text();
            QStringList parcalar = currentText.split(" : ");
            if (parcalar.size() == 2) {
                QString baslik = parcalar[0];
                bool ok;
                QString icerik = QInputDialog::getText(this, "Düzenle", "Komut İçeriği:", QLineEdit::Normal, parcalar[1], &ok);
                if (ok && !icerik.isEmpty()) {
                    QString dosyaYolu = komutKlasoru + baslik;
                    QFile komutDosyasi(dosyaYolu);
                    if (komutDosyasi.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&komutDosyasi);
                        out << icerik;
                        komutDosyasi.close();
                    }
                    currentItem->setText(baslik + " : " + icerik);
                }
            }
        } else {
            QMessageBox::warning(this, "Düzenle", "Öncelikle bir öğe seçin.");
        }
    }

    void komutCalistir() {
        QListWidgetItem *currentItem = listeWidget->currentItem();
        if (currentItem) {
            QString currentText = currentItem->text();
            QStringList parcalar = currentText.split(" : ");
            if (parcalar.size() == 2) {
                QString komut = parcalar[1];

                currentItem->setBackgroundColor(Qt::yellow);

                QProcess process;
                process.start(komut);
                process.waitForFinished();
                QString output = process.readAllStandardOutput();
                QString error = process.readAllStandardError();

                if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
                    currentItem->setBackgroundColor(Qt::green);
                } else {
                    currentItem->setBackgroundColor(Qt::red);
                }

                outputTextEdit->setPlainText(output.isEmpty() ? error : output);
            }
        } else {
            QMessageBox::warning(this, "Çalıştır", "Öncelikle bir komut seçin.");
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Pencere pencere;
    pencere.resize(600, 400);
    pencere.show();

    return app.exec();
}
