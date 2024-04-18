#ifndef GEXMAYA_GRAPHVIEW_H
#define GEXMAYA_GRAPHVIEW_H

#include "Gex/include/Gex.h"
#include "Gex/ui/include/GraphView.h"
#include <QDialog>

namespace GexMaya
{
    class GraphWindow: public QDialog
    {
        Gex::CompoundNode* cmp;
        Gex::Ui::GraphView* graphView;
    public:
        GraphWindow(Gex::CompoundNode* cmp,
                    QWidget* parent=nullptr);
    };
}


#endif //GEXMAYA_GRAPHVIEW_H
