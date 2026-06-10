#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_BTNSubmit_clicked();
    void ChangeToLookUp();
    void ChangeToMain();
    void ChangeToAdjustments();
    void ChangeToAdmin();
    void GetFullProducts();
    void FilterProductsByName();
    void FilterProductsBySku();
    void SearchProducts();
    void FilterProductsByBaked();
    void AdjustDatabase();
    void CreateProduct();
    void DeleteProduct();

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager *manager;
};
#endif // MAINWINDOW_H
