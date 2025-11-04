#ifndef MAFENETRE_H
#define MAFENETRE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MaFenetre;
}
QT_END_NAMESPACE

class MaFenetre : public QWidget
{
    Q_OBJECT

public:
    MaFenetre(QWidget *parent = nullptr);
    ~MaFenetre();

private slots:
    void on_Connect_clicked();
    void on_disconnect_clicked();
    void on_select_clicked();
    void on_Quitter_clicked();
    void on_update_clicked();

    void on_test_clicked();


private:
    Ui::MaFenetre *ui;
    void connect();
    void disconnect();
    void selectionner_carte();
    void quitter();
    void update();
};
#endif // MAFENETRE_H
