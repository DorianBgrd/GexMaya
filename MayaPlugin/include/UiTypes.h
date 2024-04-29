#ifndef GEXMAYA_UITYPES_H
#define GEXMAYA_UITYPES_H

#include "maya/MVector.h"
#include "maya/MPoint.h"
#include "maya/MFloatPoint.h"
#include "maya/MFloatVector.h"

#include "UiTsys/include/uitsys.h"

#include <QDoubleSpinBox>

namespace GexMaya
{
    template<typename T>
    class Numeric3Init: public UiTSys::TypedInitWidget
    {
        QDoubleSpinBox* xsb = nullptr;
        QDoubleSpinBox* ysb = nullptr;
        QDoubleSpinBox* zsb = nullptr;

    public:
        QWidget* CreateInitWidget() override
        {
            QWidget* widget = new QWidget();
            QHBoxLayout* layout = new QHBoxLayout();
            widget->setLayout(layout);

            xsb = new QDoubleSpinBox(this);
            xsb->setDecimals(5);
            layout->addWidget(xsb);

            ysb = new QDoubleSpinBox(this);
            ysb->setDecimals(5);
            layout->addWidget(ysb);

            zsb = new QDoubleSpinBox(this);
            zsb->setDecimals(5);
            layout->addWidget(zsb);

            return widget;
        }

        std::any CreateValue() override
        {
            T value(xsb->value(), ysb->value(), zsb->value());
            return std::make_any<T>(value);
        }
    };

    template<typename T>
    class Numeric3Widget: public UiTSys::TypedWidget
    {
        QDoubleSpinBox* xsb = nullptr;
        QDoubleSpinBox* ysb = nullptr;
        QDoubleSpinBox* zsb = nullptr;

    public:
        QWidget* CreateTypedWidget() override
        {
            QWidget* widget = new QWidget();
            QHBoxLayout* layout = new QHBoxLayout();
            widget->setLayout(layout);

            xsb = new QDoubleSpinBox(this);
            xsb->setDecimals(5);
            layout->addWidget(xsb);

            ysb = new QDoubleSpinBox(this);
            ysb->setDecimals(5);
            layout->addWidget(ysb);

            zsb = new QDoubleSpinBox(this);
            zsb->setDecimals(5);
            layout->addWidget(zsb);

            return widget;
        }

        void SetValue(std::any value) override
        {
            T v = std::any_cast<T>(value);

            xsb->setValue(v.x);
            ysb->setValue(v.y);
            zsb->setValue(v.z);
        }

        std::any GetValue() const override
        {
            T value(xsb->value(), ysb->value(), zsb->value());
            return std::make_any<T>(value);
        }

        void ShowConnected(bool connected) override
        {

        }

        void ShowDisabled(bool disabled) override
        {
            xsb->setDisabled(disabled);
            ysb->setDisabled(disabled);
            zsb->setDisabled(disabled);
        }
    };



    class MVectorInitW: public Numeric3Init<MVector>
    {};

    GENERATE_DEFAULT_INIT_WIDGET_CREATOR(MVectorInitWCreator, MVectorInitW)

    class MVectorWidget: public Numeric3Widget<MVector>
    {};

    GENERATE_DEFAULT_WIDGET_CREATOR(MVectorWidgetCreator, MVectorWidget)




    class MFloatVectorInitW: public Numeric3Init<MFloatVector>
    {};

    GENERATE_DEFAULT_INIT_WIDGET_CREATOR(MFloatVectorInitWCreator, MFloatVectorInitW)

    class MFloatVectorWidget: public Numeric3Widget<MFloatVector>
    {};

    GENERATE_DEFAULT_WIDGET_CREATOR(MFloatVectorWidgetCreator, MFloatVectorWidget)




    class MPointInitW: public Numeric3Init<MPoint>
    {};

    GENERATE_DEFAULT_INIT_WIDGET_CREATOR(MPointInitWCreator, MPointInitW)

    class MPointWidget: public Numeric3Widget<MPoint>
    {};

    GENERATE_DEFAULT_WIDGET_CREATOR(MPointWidgetCreator, MPointWidget)




    class MFloatPointInitW: public Numeric3Init<MFloatPoint>
    {};

    GENERATE_DEFAULT_INIT_WIDGET_CREATOR(MFloatPointInitWCreator, MFloatPointInitW)

    class MFloatPointWidget: public Numeric3Widget<MFloatPoint>
    {};

    GENERATE_DEFAULT_WIDGET_CREATOR(MFloatPointWidgetCreator, MFloatPointWidget)
}


#endif //GEXMAYA_UITYPES_H
