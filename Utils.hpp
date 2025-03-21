#include <QWidget>
#include <QLayout>
#include <QHBoxLayout>

class HorizontalLayout{
    public:
    HorizontalLayout(QWidget* parent){
        layout = new QHBoxLayout(parent);
    }
    QHBoxLayout* Get() { return layout; }

    private:
    QHBoxLayout* layout;
};