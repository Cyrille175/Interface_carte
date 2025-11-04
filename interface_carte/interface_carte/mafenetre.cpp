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
    connect();
}

void MaFenetre::on_disconnect_clicked()
{
    disconnect();
}

void MaFenetre::on_select_clicked()
{
    selectionner_carte();
}

void MaFenetre::on_Quitter_clicked()
{
    quitter();
}

void MaFenetre::on_update_clicked()
{
    update();
}







void MaFenetre::connect()
{
    int16_t status = MI_OK;
    MonLecteur.Type = ReaderCDC;
    MonLecteur.device = 0;
    status = OpenCOM(&MonLecteur);

    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(10);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON);
    qDebug() << "OpenCOM" << status;

    Version(&MonLecteur);
    ui->Affichage->setText(MonLecteur.version);
    ui->Affichage->update();
    LEDBuzzer(&MonLecteur, LED_RED_ON+LED_GREEN_OFF);
}




void MaFenetre::disconnect()
{
    int16_t status = RF_Power_Control(&MonLecteur, FALSE, 0);
    if (status != MI_OK) {
        ui->Affichage->setText(QString("Erreur RF OFF (status=%1)").arg(status));
    }

    LEDBuzzer(&MonLecteur, LED_OFF);

    status = CloseCOM(&MonLecteur);
    if (status != MI_OK) {
        ui->Affichage->setText(QString("Erreur CloseCOM (status=%1)").arg(status));
        return;
    }


    ui->unites->clear();
    ui->Affichage->setText("Lecteur déconnecté.");
}





void MaFenetre::selectionner_carte()
{
    uint16_t status = MI_OK;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    uint32_t pvalue = 0;

    status = ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
    if(status == MI_OK){
        LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
        DELAYS_MS(10);
        LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON);

        uint8_t data[16];
        status = Mf_Classic_Read_Block(&MonLecteur, true, 9, data, true, 2 );
        if(status == MI_OK){
            QString prenom;
            for(int i=0 ; i<16 ; i++){

                if(data[i] == 0){
                    break;
                }
                prenom.append((char)(data[i]));
            }
            ui->prenom->setText(prenom);
        }else{
            qDebug() << "Erreur: impossible de lire le prenom";
        }

        status = Mf_Classic_Read_Block(&MonLecteur, true, 10, data, true, 2 );
        if(status == MI_OK){
            QString nom;
            for(int i=0 ; i<16 ; i++){

                if(data[i] == 0){
                    break;
                }
                nom.append((char)(data[i]));
            }
            ui->nom->setText(nom);
        }else{
            qDebug() << "Erreur: impossible de lire le nom";
        }

        status = Mf_Classic_Read_Value(&MonLecteur, TRUE, 14, &pvalue, AuthKeyA, 3);
        if(status == MI_OK){
            ui->unites->setText(QString::number(pvalue));

        }else{
            qDebug() << "Erreur: impossible de lire le montant sur la carte";
        }



        LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
        DELAYS_MS(500);
        LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON);

    }
}



void MaFenetre::quitter()
{
    int16_t status = MI_OK;
    RF_Power_Control(&MonLecteur, FALSE, 0);
    status = LEDBuzzer(&MonLecteur, LED_OFF);
    status = CloseCOM(&MonLecteur);
    qApp->quit();
}


void MaFenetre::update()
{
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;

    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(1);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON);

    ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
    char DataIn1[16];
    strncpy(DataIn1, ui->nom->toPlainText().toUtf8().data(), 16);
    Mf_Classic_Write_Block(&MonLecteur, TRUE, 10, (uint8_t*)DataIn1, AuthKeyB, 2 );

    char DataIn[16];
    strncpy(DataIn, ui->prenom->toPlainText().toUtf8().data(), 16);
    Mf_Classic_Write_Block(&MonLecteur, TRUE, 9, (uint8_t*)DataIn, AuthKeyB, 2 );
}

