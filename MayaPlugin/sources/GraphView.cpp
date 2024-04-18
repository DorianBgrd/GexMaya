#include "MayaPlugin/include/GraphView.h"

#include <QVBoxLayout>
#include <QFile>


GexMaya::GraphWindow::GraphWindow(Gex::CompoundNode* c,
      QWidget* parent): QDialog(parent)
{
    Gex::Ui::NodeItem::SetDefaultWidth(225);
    Gex::Ui::NodeItem::SetDefaultSpacing(10);
    Gex::Ui::NodeItem::SetDefaultFooter(10);
    Gex::Ui::NodeItem::SetDefaultTitleOffset(30);

    QFile styleFile("D:\\WORK\\GEX\\Gex\\ui\\soft\\resources\\stylesheet.css");
    styleFile.open(QFile::ReadOnly);
    QString style = styleFile.readAll();

    setStyleSheet(style);

    cmp = c;

    auto* layout = new QVBoxLayout();
    setLayout(layout);

    graphView = new Gex::Ui::GraphView(c, this);
    layout->addWidget(graphView);
}