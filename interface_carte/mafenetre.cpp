#include "mafenetre.h"
#include "ui_mafenetre.h"
#include <QDebug>
#include <array>
#include <algorithm>
#include <QMessageBox>
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
    ui->setupUi(this); // initialisation UI

    // style de l'interface
    this->setStyleSheet(
        "QWidget#MaFenetre { background-color: #E8F0FE; }"
        "QLineEdit { background-color: #FFFFFF; border: 1px solid #B0C4DE; border-radius: 4px; }"
        "QPushButton { background-color: #A7C7E7; border-radius: 6px; }"
        "QPushButton:hover { background-color: #89ABE3; }"
        );

}

MaFenetre::~MaFenetre()
{
    delete ui;
}


ReaderName MonLecteur; // structure décrivant le lecteur


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

void MaFenetre::on_charger_clicked()
{
    charger();
}




void MaFenetre::connect()
{
    LIB_VERSION Versioni;
    int16_t status = GetLibraryExtension(&Versioni); // récupérer version de la librairie
    MonLecteur.Type = ReaderCDC;
    MonLecteur.device = 0;

    if (status != MI_OK)
    {
        qDebug() << "Erreur sur GetLibraryExtension";
        return;
    }

    status = OpenCOM(&MonLecteur); // ouvrir la COM
    if (status != MI_OK)
    {
        qDebug() << "Erreur sur OpenCOM";
        return;
    }

    status = Version(&MonLecteur); // récupérer version firmware
    if (status != MI_OK)
    {
        qDebug() << "Erreur sur Version";
        return;
    }

    status = RF_Power_Control(&MonLecteur,TRUE,0); // activer RF

    qDebug() << "OpenCOM" << status;

    ui->Affichage->setText(MonLecteur.version); // afficher version
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(10);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON);
}




void MaFenetre::disconnect()
{
    int16_t status = RF_Power_Control(&MonLecteur, FALSE, 0); // désactiver RF
    if (status != MI_OK) {
        qDebug() << "Erreur sur le RF_Power_Control";
    }

    LEDBuzzer(&MonLecteur, LED_OFF);

    status = CloseCOM(&MonLecteur); // fermer COM
    if (status != MI_OK) {
        qDebug() << "Erreur sur le CloseCOM";
        return;
    }

    // nettoyer l'interface après déconnexion
    ui->nom->clear();
    ui->prenom->clear();
    ui->unites->clear();
    ui->increment->clear();
    ui->decrement->clear();
    ui->Affichage->setText("Lecteur déconnecté.");
}



void MaFenetre::selectionner_carte()
{
    int16_t status = MI_OK;

    uint8_t atq[2] = {0};
    uint8_t sak = 0;
    uint8_t uid[10] = {0};
    uint16_t uid_len = 0;

    // détecter carte ISO14443-A
    status = ISO14443_3_A_PollCard(&MonLecteur, atq, &sak, uid, &uid_len);
    if (status != MI_OK) {
        ui->Affichage->setText("Aucune carte detectée (PollCard échouué)");
         QMessageBox::critical(this, "Erreur", "Aucune carte a proximité");
        qDebug() << "ISO14443_3_A_PollCard status" << status;
        return;
    }

    qDebug() << "Carte detecté";
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(1);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON);

    // lire bloc 9 -> prénom
    uint8_t data[16];
    status = Mf_Classic_Read_Block(&MonLecteur, true, 9, data, true, 2 );
    if(status == MI_OK){
        QString prenom;
        for(int i=0 ; i<16 ; i++){
            if(data[i] == 0) break; // fin de chaîne
            prenom.append((char)(data[i]));
        }
        ui->prenom->setText(prenom);
    } else {
        qDebug() << "Impossible de lire le prenom";
    }

    // lire bloc 10 -> nom
    status = Mf_Classic_Read_Block(&MonLecteur, true, 10, data, true, 2 );
    if(status == MI_OK){
        QString nom;
        for(int i=0 ; i<16 ; i++){
            if(data[i] == 0) break;
            nom.append((char)(data[i]));
        }
        ui->nom->setText(nom);
    } else {
        qDebug() << "Impossible de lire le nom";
    }

    // lire valeur (bloc 14)
    uint32_t value = 0;
    status = Mf_Classic_Read_Value(&MonLecteur, TRUE, 14, &value, AuthKeyA, 3);
    if(status == MI_OK){
        ui->unites->setText(QString::number(value));
    } else {
        qDebug() << "Impossible de lire la valeur des unites dans le compte";
    }

    // indication lecture
    LEDBuzzer(&MonLecteur, LED_GREEN_ON);
    DELAYS_MS(500);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON);
}




void MaFenetre::quitter()
{
    int16_t status = MI_OK;
    RF_Power_Control(&MonLecteur, FALSE, 0); // couper RF
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

    //Verifier que la carte est toujours bien connectée
    int16_t status = MI_OK;
    status = ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
    if (status != MI_OK)
    {
        QMessageBox::critical(this, "Erreur d'ecriture", "Veuillez reonnecter la carte au lecteur");
    }

    // indication écriture
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(1);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON);

    ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len); // vérifier carte
    char DataIn1[16];
    strncpy(DataIn1, ui->nom->toPlainText().toUtf8().data(), 16); // mettre le nom dans un buffer
    Mf_Classic_Write_Block(&MonLecteur, TRUE, 10, (uint8_t*)DataIn1, AuthKeyB, 2 ); // écrire bloc 10

    char DataIn[16];
    strncpy(DataIn, ui->prenom->toPlainText().toUtf8().data(), 16); // mettre le prenom dans un buffer
    Mf_Classic_Write_Block(&MonLecteur, TRUE, 9, (uint8_t*)DataIn, AuthKeyB, 2 ); // écrire bloc 9

}


void MaFenetre::payer(){
    int16_t status = MI_OK;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    uint32_t value = 0;

    uint32_t Valeur_Decrementer = ui->decrement->value(); // montant à retirer

    //Verifier que la carte est toujours bien connectée
    status = ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
    if (status != MI_OK)
    {
        QMessageBox::critical(this, "Erreur d'ecriture", "Veuillez reonnecter la carte au lecteur");
    }


    // décrémenter puis restaurer la valeur
    Mf_Classic_Decrement_Value(&MonLecteur, TRUE, 14, Valeur_Decrementer, 13, AuthKeyA, 3); // Cette fonction me permet de prendre la valeur dans le bloc 14 la decrementer et placer le resultat dans le bloc 13
    Mf_Classic_Restore_Value(&MonLecteur, TRUE, 13, 14, AuthKeyA, 3); // je met le contenu du bloc 13 dans le bloc 14

    Mf_Classic_Read_Value(&MonLecteur, TRUE, 14, &value, AuthKeyA, 3);
    ui->unites->setText(QString::number(value)); // mise à jour affichage

    // indication écriture
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(1);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON);
}



void MaFenetre::charger()
{
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    uint32_t value = 0;

    //Verifier que la carte est toujours bien connectée
    int16_t status = MI_OK;
    status = ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);
    if (status != MI_OK)
    {
        QMessageBox::critical(this, "Erreur d'ecriture", "Veuillez reonnecter la carte au lecteur");
    }

    uint32_t Valeur_Increment = ui->increment->value(); // montant à ajouter
    ISO14443_3_A_PollCard(&MonLecteur, atq, sak, uid, &uid_len);

    // incrémenter puis restaurer la valeur
    Mf_Classic_Increment_Value(&MonLecteur, TRUE, 14, Valeur_Increment, 13, AuthKeyB, 3);
    Mf_Classic_Restore_Value(&MonLecteur, TRUE, 13, 14, AuthKeyB, 3);

    Mf_Classic_Read_Value(&MonLecteur, TRUE, 14, &value, AuthKeyA, 3);
    ui->unites->setText(QString::number(value)); // mise à jour affichage

    // indication écriture
    LEDBuzzer(&MonLecteur, LED_GREEN_ON+LED_YELLOW_ON+LED_RED_ON+LED_GREEN_ON);
    DELAYS_MS(1);
    LEDBuzzer(&MonLecteur, LED_GREEN_ON);
}
