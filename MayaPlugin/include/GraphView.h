#ifndef GEXMAYA_GRAPHVIEW_H
#define GEXMAYA_GRAPHVIEW_H

#include "Gex/include/Gex.h"
#include "Gex/ui/include/GraphView.h"
#include <functional>
#include <QDialog>

namespace GexMaya
{
    typedef std::function<void()> CloseCallback;

    class GraphWindow: public QDialog
    {
        Gex::CompoundNode* cmp;
        Gex::Ui::GraphView* graphView;
        CloseCallback closeCallback;
    public:
        GraphWindow(Gex::CompoundNode* cmp,
                    CloseCallback callback,
                    QWidget* parent=nullptr);

        void closeEvent(QCloseEvent* event) override;
    };
}


#endif //GEXMAYA_GRAPHVIEW_H
