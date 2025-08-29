#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Hello Qt with Sail!");
    window.setFixedSize(300, 200);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    
    QLabel *label = new QLabel("Welcome to Sail + Qt!");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    QPushButton *button = new QPushButton("Click Me!");
    layout->addWidget(button);

    QObject::connect(button, &QPushButton::clicked, [&label]() {
        label->setText("Button clicked!");
    });

    window.show();
    return app.exec();
}