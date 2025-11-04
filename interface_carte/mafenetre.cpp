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
    update ();
}

void MaFenetre::on_payer_clicked()
{
    payer();
}







void MaFenetre::connect()
{
    LIB_VERSION Versioni;
    BOOL RFOnOff = TRUE;                   /**< [In] Activation ou desactivation . */
    uint8_t Delay = 0;
    int16_t status = GetLibraryExtension(&Versioni);
    MonLecteur.Type = ReaderCDC;
    MonLecteur.device = 0;

    if (status != MI_OK)
    {
        qDebug() << "Erreur sur GetLibraryExtension";
        return;
    }
    status = OpenCOM(&MonLecteur);

    if (status != MI_OK)
    {
        qDebug() << "Erreur sur OpenCOM";
        return;
    }

    status = Version(&MonLecteur);

    if (status != MI_OK)
    {
        qDebug() << "Erreur sur Version";
        return;
    }

    status = RF_Power_Control(&MonLecteur,RFOnOff,Delay);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(10);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON);
    qDebug() << "OpenCOM" << status;

    ui->Affichage->setText(MonLecteur.version);
    ui->Affichage->update();
    LEDBuzzer(&MonLecteur, LED_RED_ON+LED_GREEN_OFF);
}




void MaFenetre::disconnect()
{
    int16_t status = RF_Power_Control(&MonLecteur, FALSE, 0);
    if (status != MI_OK) {
        qDebug() << "Erreur sur le RF_Power_Control";
    }

    LEDBuzzer(&MonLecteur, LED_OFF);

    status = CloseCOM(&MonLecteur);
    if (status != MI_OK) {
        qDebug() << "Erreur sur le CloseCOM";
        return;
    }


    ui->unites->clear();
    ui->Affichage->setText("Lecteur déconnecté.");
}



void MaFenetre::selectionner_carte()
{
    int16_t status = MI_OK;

    uint8_t atq[2] = {0};
    uint8_t sak = 0;
    uint8_t uid[10] = {0};
    uint16_t uid_len = 0;



    status = ISO14443_3_A_PollCard(&MonLecteur, atq, &sak, uid, &uid_len);
    if (status != MI_OK) {
        ui->Affichage->setText("Aucune carte detectée (PollCard échouué)");
        qDebug() << "ISO14443_3_A_PollCard status" << status;
        return;
    }
        qDebug() << "Carte detecté";
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
            qDebug() << "Impossible de lire le prenom";
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
            qDebug() << "Impossible de lire le nom";
        }

        uint32_t value = 0;

        status = Mf_Classic_Read_Value(&MonLecteur, TRUE, 14, &value, AuthKeyA, 3);
        if(status == MI_OK){
            ui->unites->setText(QString::number(value));

        }else{
            qDebug() << "Impossible de lire la valeur des unites dans le compte";
        }
        LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
        DELAYS_MS(500);
        LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON);
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


void MaFenetre::payer(){

    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    uint32_t value = 0;

    uint32_t Valeur_Decrementer = 0;
    Valeur_Decrementer = ui->decrement->value();
    ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
    Mf_Classic_Decrement_Value(&MonLecteur, TRUE, 14, Valeur_Decrementer, 13, AuthKeyA, 3);
    Mf_Classic_Restore_Value(&MonLecteur, TRUE, 13, 14, AuthKeyA, 3);
    Mf_Classic_Read_Value(&MonLecteur, TRUE, 14, &value, AuthKeyA, 3);
    ui->unites->setText(QString::number(value));



    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(500);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON);


}

