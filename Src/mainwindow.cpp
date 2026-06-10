#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    // Allows the application to make http requests
    manager = new QNetworkAccessManager(this);

    ui->setupUi(this);

    // Apply pointing hand cursor to all buttons
    for (QPushButton* btn : this->findChildren<QPushButton*>()) {
        btn->setCursor(Qt::PointingHandCursor);
    }

    // Simple UI as this is an employee only application
    this->setStyleSheet(
        "QMainWindow { background-color: #5388bd; }"
        );

    qApp->setStyleSheet(
        "QPushButton {"
        "    background-color: #95b2cf;"
        "    border-style: solid;"
        "    color: #000000;"
        "    cursor: pointer;"
        "}"

        "QLineEdit {"
        "    border-style: solid;"
        "    border-width: 1px;"
        "}"
    );

    // Set Default page as the main page on start up
    ui->stackedWidget->setCurrentWidget(ui->MainPage);

    //QObject::connect(Object emmitting signal, name of signal, object that will recieve the signal, function to perform)
    QObject::connect(ui->LookUpBtn, &QPushButton::clicked, this, &MainWindow::ChangeToLookUp);
    QObject::connect(ui->AdjHomeBtn, &QPushButton::clicked, this, &MainWindow::ChangeToMain);
    QObject::connect(ui->HomeBtn2, &QPushButton::clicked, this, &MainWindow::ChangeToMain);
    QObject::connect(ui->AdjustmentBtn, &QPushButton::clicked, this, &MainWindow::ChangeToAdjustments);
    QObject::connect(ui->SearchBtn, &QPushButton::clicked, this, &MainWindow::SearchProducts);
    QObject::connect(ui->AdjustSubmitBtn, &QPushButton::clicked, this, &MainWindow::AdjustDatabase);
    QObject::connect(ui->AdminPageBtn, &QPushButton::clicked, this, &MainWindow::ChangeToAdmin);
    QObject::connect(ui->AdminHomeBtn, &QPushButton::clicked, this, &MainWindow::ChangeToMain);
    QObject::connect(ui->AdminCreateBtn, &QPushButton::clicked, this, &MainWindow::CreateProduct);
    QObject::connect(ui->AdminDeleteBtn, &QPushButton::clicked, this, &MainWindow::DeleteProduct);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Not used
void MainWindow::on_BTNSubmit_clicked()
{
    // TODO: handle submit
}

// Change page to look up page
void MainWindow::ChangeToLookUp(){
    ui->stackedWidget->setCurrentWidget(ui->LookUpPage);
}

// Change page to the main page
void MainWindow::ChangeToMain(){
    // Clear all input and output boxes
    ui->OutputField->setText("Result: ");
    ui->AdjOutput->setText("Result: ");
    ui->BakedInput->setText("");
    ui->AdjBakedInput->setText("");
    ui->NameInput->setText("");
    ui->SkuInput->setText("");
    ui->AdjBakedInput->setText("");
    ui->AdjOnHandInput->setText("");
    ui->AdjSkuInput->setText("");

    ui->stackedWidget->setCurrentWidget(ui->MainPage);
}

// Change page to adjustments page
void MainWindow::ChangeToAdjustments(){
    ui->stackedWidget->setCurrentWidget(ui->AdjustmentPage);
}

// Change page to admin page
void MainWindow::ChangeToAdmin(){
    ui->stackedWidget->setCurrentWidget(ui->AdminPage);
    ui->AdminNameInput->setText("");
    ui->AdminBakedInput->setText("");
    ui->AdminImageInput->setText("");
    ui->AdminPriceInput->setText("");
    ui->AdminSkuInput->setText("");
    ui->AdminQuantityInput->setText("");
    ui->AdminDelInput->setText("");
    ui->AdminOutput->setText("Output:\n");
}

// Print full product list to debug console
void MainWindow::GetFullProducts(){
    QUrl url("http://localhost:5000/products");

    // Wraps the url in a request object that can have headers, cookies, tokens, etc
    QNetworkRequest request(url);

    // This tells the server to expect json data, get requests dont send a body, some APIs expect a header still
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // manager->get() sends the http GET request asynchronously
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Checks for network errors such as no internet, server offline, 404/505 error, ssl issues
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Network error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        // Returns raw bytes returned by server Example: [{"name":"Keyboard","price":29.99}]
        QByteArray response = reply->readAll();
        // Prints the response
        qDebug() << "API Response:" << response;

        // Converts the raw bytes in response to a json format
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        // For each element in the array make it an object
        for (const QJsonValue &v : arr) {
            QJsonObject obj = v.toObject();
            qDebug() << "Product:" << obj["name"].toString()
                     << "Price:" << obj["price"].toDouble();
        }

        // Prevent memory leaks by deleting the reply object after the event loop finishes
        reply->deleteLater();
    });
}

// Checks all input boxes for data then run correct function
void MainWindow::SearchProducts(){
    QString skuInputField = ui->SkuInput->text();
    QString nameInputField = ui->NameInput->text();
    QString bakedInputField = ui->BakedInput->text();

    // Find which field has input, sku has top priority, then name, then baked. If no input given display error
    if (skuInputField != ""){
        FilterProductsBySku();
    } else if (nameInputField != ""){
        FilterProductsByName();
    } else if (bakedInputField != ""){
        FilterProductsByBaked();
    } else {
        ui->OutputField->setText("Result:\nNo input given");
    }
}

// Output data by filtering product list by given name
void MainWindow::FilterProductsByName(){
    QUrl url("http://localhost:5000/products");

    // Wraps the url in a request object that can have headers, cookies, tokens, etc
    QNetworkRequest request(url);

    // This tells the server to expect json data, get requests dont send a body, some APIs expect a header still
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // manager->get() sends the http GET request asynchronously
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Checks for network errors such as no internet, server offline, 404/505 error, ssl issues
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Network error:" << reply->errorString();
            ui->OutputField->setText("Result:\nUnable to connect");
            reply->deleteLater();
            return;
        }

        // Returns raw bytes returned by server Example: [{"name":"Keyboard","price":29.99}]
        QByteArray response = reply->readAll();
        // Prints the response
        qDebug() << "API Response:" << response;

        // Converts the raw bytes in response to a json format
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        QString searchName = ui->NameInput->text();
        bool productFound = false;

        for (const QJsonValue &v : arr) {
            QJsonObject obj = v.toObject();

            QString name = obj["name"].toString();
            QString sku = obj["sku"].toString();
            QString baked = obj["baked"].toString();

            // Find item with given name
            if (name.compare(searchName, Qt::CaseInsensitive) == 0) {
                double price = obj["price"].toDouble();
                double quantity = obj["quantity"].toDouble();

                qDebug() << "Match found:" << name
                         << "Price:" << obj["price"].toDouble();
                ui->OutputField->setText("Results:\n-Found by Name\nSku: " +
                                         sku +
                                         "\nProduct Name: " +
                                         name + "\nPrice: " +
                                         QString::number(price) +
                                         "\nQuantity: " +
                                         QString::number(quantity) +
                                         "\nLast Baked On: " +
                                         baked);
                productFound = true;
            }
        }

        if(!productFound) {
            FilterProductsByBaked();
        }

        // Prevent memory leaks by deleting the reply object after the event loop finishes
        reply->deleteLater();
    });
}

// Output data by filtering product list by given sku
void MainWindow::FilterProductsBySku(){
    QUrl url("http://localhost:5000/products");

    // Wraps the url in a request object that can have headers, cookies, tokens, etc
    QNetworkRequest request(url);

    // This tells the server to expect json data, get requests dont send a body, some APIs expect a header still
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // manager->get() sends the http GET request asynchronously
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Checks for network errors such as no internet, server offline, 404/505 error, ssl issues
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Network error:" << reply->errorString();
            ui->OutputField->setText("Result:\nUnable to connect");
            reply->deleteLater();
            return;
        }

        // Returns raw bytes returned by server Example: [{"name":"Keyboard","price":29.99}]
        QByteArray response = reply->readAll();
        // Prints the response
        qDebug() << "API Response:" << response;

        // Converts the raw bytes in response to a json format
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        QString searchSku = ui->SkuInput->text();

        bool productFound = false;

        for (const QJsonValue &v : arr) {
            QJsonObject obj = v.toObject();

            QString name = obj["name"].toString();
            QString sku = obj["sku"].toString();
            QString baked = obj["baked"].toString();

            // Find item with given sku
            if (sku.compare(searchSku, Qt::CaseInsensitive) == 0) {
                double price = obj["price"].toDouble();
                double quantity = obj["quantity"].toDouble();

                qDebug() << "Match found:" << name
                         << "Price:" << obj["price"].toDouble();
                ui->OutputField->setText("Results:\n-Found by Sku\nSku: " +
                                        sku +
                                        "\nProduct Name: " +
                                        name + "\nPrice: " +
                                        QString::number(price) +
                                        "\nQuantity: " +
                                        QString::number(quantity) +
                                        "\nLast Baked On: " +
                                        baked);
                productFound = true;
            }
        }

        if (!productFound) {
            FilterProductsByName();
        }

        // Prevent memory leaks by deleting the reply object after the event loop finishes
        reply->deleteLater();
    });
}

// Output data by filtering product list by given baked date
void MainWindow::FilterProductsByBaked(){
    QUrl url("http://localhost:5000/products");

    // Wraps the url in a request object that can have headers, cookies, tokens, etc
    QNetworkRequest request(url);

    // This tells the server to expect json data, get requests dont send a body, some APIs expect a header still
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // manager->get() sends the http GET request asynchronously
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Checks for network errors such as no internet, server offline, 404/505 error, ssl issues
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Network error:" << reply->errorString();
            ui->OutputField->setText("Result:\nUnable to connect");
            reply->deleteLater();
            return;
        }

        // Returns raw bytes returned by server Example: [{"name":"Keyboard","price":29.99}]
        QByteArray response = reply->readAll();
        // Prints the response
        qDebug() << "API Response:" << response;

        // Converts the raw bytes in response to a json format
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        QString searchBaked = ui->BakedInput->text();
        bool productFound = false;

        for (const QJsonValue &v : arr) {
            QJsonObject obj = v.toObject();

            QString name = obj["name"].toString();
            QString sku = obj["sku"].toString();
            QString baked = obj["baked"].toString();

            // Find item with given sku
            if (baked.compare(searchBaked, Qt::CaseInsensitive) == 0) {
                double price = obj["price"].toDouble();
                double quantity = obj["quantity"].toDouble();

                ui->OutputField->setText("Results:\n-Found by Baked on Date\nSku: " +
                                         sku +
                                         "\nProduct Name: " +
                                         name + "\nPrice: " +
                                         QString::number(price) +
                                         "\nQuantity: " +
                                         QString::number(quantity) +
                                         "\nLast Baked On: " +
                                         baked);
                productFound = true;
            }
        }

        if (!productFound) {
            ui->OutputField->setText("Result:\nProduct not found");
        }

        // Prevent memory leaks by deleting the reply object after the event loop finishes
        reply->deleteLater();
    });
}

// Adjust the database with the given data, sku required
void MainWindow::AdjustDatabase(){
    QString sku = ui->AdjSkuInput->text();
    QString onHand = ui->AdjOnHandInput->text();

    if(sku != "" && onHand != ""){

        QString url = QString("http://localhost:5000/products/%1").arg(sku);

        // Wraps the url in a request object that can have headers, cookies, tokens, etc
        QNetworkRequest request(url);

        // This tells the server to expect json data, get requests dont send a body, some APIs expect a header still
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        // Build JSON body
        QJsonObject json;
        json["sku"] = sku;
        json["quantity"] = onHand.toInt();
        QByteArray body = QJsonDocument(json).toJson();

        QNetworkReply *reply = manager->sendCustomRequest(request, "PATCH", body);
\
        // Waits for a response and then execute code
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {

            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "PATCH error:" << reply->errorString();
                ui->OutputField->setText("Result:\nUnable to update item");
                reply->deleteLater();
                return;
            }

            ui->AdjOutput->setText("Result:\nUpdate succesfull");

            // Prevent memory leaks by deleting the reply object after the event loop finishes, runs after a confirmed response above
            reply->deleteLater();
        });
    }
}

// Create a new product and give it default values
void MainWindow::CreateProduct(){
    QUrl url("http://localhost:5000/products");

    // Wraps the url in a request object that can have headers, cookies, tokens, etc
    QNetworkRequest request(url);

    // This tells the server to expect json data, get requests dont send a body, some APIs expect a header still
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["sku"] = ui->AdminSkuInput->text();
    json["name"] = ui->AdminNameInput->text();
    json["quantity"] = ui->AdminQuantityInput->text().toInt();
    json["image"] = ui->AdminImageInput->text();
    json["baked"] = ui->AdminBakedInput->text();
    json["price"] = ui->AdminPriceInput->text();

    // Creates a byte array with the json document created above
    QByteArray body = QJsonDocument(json).toJson();

    QNetworkReply *reply = manager->post(request, body);

    // Waits for a response and then execute code
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Post error:" << reply->errorString();
            ui->OutputField->setText("Cant connect to servers");
            reply->deleteLater();
            return;
        }

        ui->AdminOutput->setText("Created item with sku: " + ui->AdminSkuInput->text());

        // Prevent memory leaks by deleting the reply object after the event loop finishes, runs after a confirmed response above
        reply->deleteLater();
    });
}

// Delete a product based on the given sku number
void MainWindow::DeleteProduct(){
    QString sku = ui->AdminDelInput->text();
    QUrl url("http://localhost:5000/products/" + sku);

    // Wraps the url in a request object that can have headers, cookies, tokens, etc
    QNetworkRequest request(url);

    // This tells the server to expect json data, get requests dont send a body, some APIs expect a header still
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Sends a delete request to the API
    QNetworkReply *reply = manager->deleteResource(request);

    // Waits for a response from the API and then execute code after response is given
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {

        // If unable to connect to server run statement
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Post error:" << reply->errorString();
            ui->OutputField->setText("Cant connect to servers");
            reply->deleteLater();
            return;
        }

        // Provide UI feedback when product is deleted
        ui->AdminOutput->setText("Deleted item with sku: " + ui->AdminDelInput->text() );

        // Prevent memory leaks by deleting the reply object after the event loop finishes, runs after a confirmed response above
        reply->deleteLater();
    });
}