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

    char* envpath = std::getenv("GEX_RESOURCES_PATH");
    if (envpath)
    {
        std::string path = envpath;
        if (!path.empty())
        {
            std::filesystem::path styleFile = path;
            styleFile.append("stylesheet.css");
            if (std::filesystem::exists(styleFile))
            {
                QFile file(styleFile.string().c_str());
                setStyleSheet(file.readAll());
            }
        }
    }

    cmp = c;

    auto* layout = new QVBoxLayout();
    setLayout(layout);

    graphView = new Gex::Ui::GraphView(cmp, this);
    layout->addWidget(graphView);
}