#include "mafenetre.h"
#include "ui_mafenetre.h"
#include <QDebug>
#include <array>
#include <algorithm>

#include "MfErrNo.h"
#include "Sw_Device.h"
#include "Sw_ISO14443A-3.h"
#include "Sw_Mf_Classic.h"
#include "Tools.h"
#include "TypeDefs.h"

MaFenetre::MaFenetre(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MaFenetre)
{
    ui->setupUi(this);
}

MaFenetre::~MaFenetre()
{
    delete ui;
}


ReaderName MonLecteur;
void MaFenetre::on_Connect_clicked()
{
    int16_t status = MI_OK;
    MonLecteur.Type = ReaderCDC;
    MonLecteur.device = 0;

    status = OpenCOM(&MonLecteur);
    qDebug() << "OpenCOM" << status;
    status = Version(&MonLecteur);
    ui->Affichage->setText(MonLecteur.version);
    ui->Affichage->update();
}

void MaFenetre::on_select_clicked()
{
    selectionner_carte();
}


void MaFenetre::selectionner_carte()
{
    int16_t status;
    uint8_t atq[2]={0}, sak=0, uid[10]={0}; uint16_t uid_len=0;

    status = ISO14443_3_A_PollCard(&MonLecteur, atq, &sak, uid, &uid_len);
    if (status != MI_OK) {
        ui->Affichage->setText("Aucune carte détectée.");
        return;
    }


    uint8_t pData[16]={0};
    uint8_t Prenom[16]={0};
    uint8_t Nom[16]={0};

    status = Mf_Classic_Read_Block(&MonLecteur, TRUE, 8, pData, AuthKeyA, 2);
    status = Mf_Classic_Read_Block(&MonLecteur, TRUE, 9, Prenom, AuthKeyA, 2);
    status = Mf_Classic_Read_Block(&MonLecteur, TRUE, 10, Nom, AuthKeyA, 2);

    QString msg = "Carte détectée.\n";
    if (status == MI_OK) {
        QString identifiant = QString::fromLatin1(reinterpret_cast<const char*>(pData), 16).trimmed();
        QString prenom = QString::fromLatin1(reinterpret_cast<const char*>(Prenom), 16).trimmed();
        QString nom = QString::fromLatin1(reinterpret_cast<const char*>(Nom), 16).trimmed();
        msg += "Bloc 8 : " + identifiant;
        msg += "\nPrenom" + prenom;
        msg += "\nNom" + nom;
    } else {
        msg += QString("Lecture bloc 8 impossible (status=%1)").arg(status);
    }

    ui->Affichage->setText(msg);
}

