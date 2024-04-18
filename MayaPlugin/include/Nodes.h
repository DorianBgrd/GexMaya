#ifndef GEXMAYA_NODES_H
#define GEXMAYA_NODES_H

#include "rapidjson/document.h"
#include "Tsys/include/tsys.h"
#include "Tsys/include/defaultTypes.h"
#include "Gex/Gex/include/Gex.h"
#include "boost/python.hpp"

#include "maya/MVector.h"
#include "maya/MMatrix.h"
#include "maya/MItGeometry.h"


namespace GexMaya
{
    template<typename T>
    class UniformMathNode: public Gex::Node
    {
        void InitAttributes() override
        {
            CreateAttribute<T>("InputA", Gex::AttrValueType::Single,
                               Gex::AttrType::Input);
            CreateAttribute<T>("InputB", Gex::AttrValueType::Single,
                               Gex::AttrType::Input);

            TSys::Enum op;
            op.AddValue(0, "Add");
            op.AddValue(1, "Substract");
            op.AddValue(2, "Multiply");
            op.AddValue(3, "Divide");

            std::any opa = op;
            CreateAttributeFromValue("Operation", opa,
                                     Gex::AttrValueType::Single,
                                     Gex::AttrType::Input);

            CreateAttribute<T>("Output", Gex::AttrValueType::Single,
                               Gex::AttrType::Output);
        }

        bool Evaluate(Gex::NodeAttributeData &ctx,
                      Gex::GraphContext &graphContext,
                      Gex::NodeProfiler &profiler) override
        {
            T value;

            T inputA = ctx.GetAttribute("InputA").GetValue<T>();
            T inputB = ctx.GetAttribute("InputB").GetValue<T>();

            auto op = ctx.GetAttribute("Operation").GetValue<TSys::Enum>();
            unsigned int operation = op.CurrentIndex();

            if (operation == 0)
                value = inputA + inputB;
            else if (operation == 1)
                value = inputA - inputB;
            else if (operation == 2)
                value = inputA * inputB;
            else
                value = inputA / inputB;

            return ctx.GetAttribute("Output").SetValue(value);
        }
    };




    template<typename T>
    class AddSubMathNode: public Gex::Node
    {
        void InitAttributes() override
        {
            CreateAttribute<T>("InputA", Gex::AttrValueType::Single,
                               Gex::AttrType::Input);
            CreateAttribute<T>("InputB", Gex::AttrValueType::Single,
                               Gex::AttrType::Input);

            TSys::Enum op;
            op.AddValue(0, "Add");
            op.AddValue(1, "Substract");

            std::any opa = op;
            CreateAttributeFromValue("Operation", opa,
                                     Gex::AttrValueType::Single,
                                     Gex::AttrType::Input);

            CreateAttribute<T>("Output", Gex::AttrValueType::Single,
                               Gex::AttrType::Output);
        }

        bool Evaluate(Gex::NodeAttributeData &ctx,
                      Gex::GraphContext &graphContext,
                      Gex::NodeProfiler &profiler) override
        {
            T value;

            T inputA = ctx.GetAttribute("InputA").GetValue<T>();
            T inputB = ctx.GetAttribute("InputB").GetValue<T>();

            auto op = ctx.GetAttribute("Operation").GetValue<TSys::Enum>();
            unsigned int operation = op.CurrentIndex();

            if (operation == 0)
                value = inputA + inputB;
            else
                value = inputA - inputB;

            return ctx.GetAttribute("Output").SetValue(value);
        }
    };


    template<typename T>
    class SingleMultMathNode: public Gex::Node
    {
        void InitAttributes() override
        {
            CreateAttribute<T>("InputA", Gex::AttrValueType::Single,
                               Gex::AttrType::Input);
            CreateAttribute<double>("InputB", Gex::AttrValueType::Single,
                                    Gex::AttrType::Input);

            TSys::Enum op;
            op.AddValue(0, "Mult");
            op.AddValue(1, "Divide");

            std::any opa = op;
            CreateAttributeFromValue("Operation", opa,
                                     Gex::AttrValueType::Single,
                                     Gex::AttrType::Input);

            CreateAttribute<T>("Output", Gex::AttrValueType::Single,
                               Gex::AttrType::Output);
        }

        bool Evaluate(Gex::NodeAttributeData &ctx,
                      Gex::GraphContext &graphContext,
                      Gex::NodeProfiler &profiler) override
        {
            T value;

            T inputA = ctx.GetAttribute("InputA").GetValue<T>();
            auto inputB = ctx.GetAttribute("InputB").GetValue<double>();

            auto op = ctx.GetAttribute("Operation").GetValue<TSys::Enum>();
            unsigned int operation = op.CurrentIndex();

            if (operation == 0)
                value = inputA * inputB;
            else
                value = inputA / inputB;

            return ctx.GetAttribute("Output").SetValue(value);
        }
    };


    class MVectorAdd: public AddSubMathNode<MVector> {};


    GENERATE_DEFAULT_BUILDER(MVectorAddBuilder, MVectorAdd)


    class MPointAdd: public AddSubMathNode<MPoint> {};


    GENERATE_DEFAULT_BUILDER(MPointAddBuilder, MPointAdd)


    class MFloatVectorAdd: public AddSubMathNode<MFloatVector> {};


    GENERATE_DEFAULT_BUILDER(MFloatVectorAddBuilder, MFloatVectorAdd)


    class MFloatPointAdd: public AddSubMathNode<MFloatPoint> {};


    GENERATE_DEFAULT_BUILDER(MFloatPointAddBuilder, MFloatPointAdd)


    class MVectorMult: public SingleMultMathNode<MVector> {};


    GENERATE_DEFAULT_BUILDER(MVectorMultBuilder, MVectorMult)


    class MPointMult: public SingleMultMathNode<MPoint> {};


    GENERATE_DEFAULT_BUILDER(MPointMultBuilder, MPointMult)


    class MFloatVectorMult: public SingleMultMathNode<MFloatVector> {};


    GENERATE_DEFAULT_BUILDER(MFloatVectorMultBuilder, MFloatVectorMult)


    class MFloatPointMult: public SingleMultMathNode<MFloatPoint> {};


    GENERATE_DEFAULT_BUILDER(MFloatPointMultBuilder, MFloatPointMult)


    class DeformerGraph: public Gex::CompoundNode
    {
        void InitAttributes() override;

        bool Evaluate(Gex::NodeAttributeData &ctx,
                      Gex::GraphContext &graphContext,
                      Gex::NodeProfiler &profiler) override;
    };

    GENERATE_DEFAULT_BUILDER(DeformerGraphBuilder, DeformerGraph)


    class GeomIter: public Gex::CompoundNode
    {
        Gex::ScheduleNodeList schelNodes;
    protected:
        void InitAttributes() override;

        bool PreEvaluate(Gex::NodeAttributeData &ctx,
                         Gex::GraphContext &graphContext,
                         Gex::NodeProfiler &profiler) override;

        bool Evaluate(Gex::NodeAttributeData &ctx,
                      Gex::GraphContext &graphContext,
                      Gex::NodeProfiler &profiler)
                      override;

        bool PostEvaluate(Gex::NodeAttributeData &ctx,
                          Gex::GraphContext &graphContext,
                          Gex::NodeProfiler &profiler) override;


        Gex::ScheduleNodeList ToScheduledNodes() override;
    };

    GENERATE_DEFAULT_BUILDER(GeomIterBuilder, GeomIter)


}


//REGISTER_PLUGIN(Gex::PluginLoader* loader)
//{
//    loader->RegisterTypeHandler<MVector, GexMayaPlugin::MVectorHandler>();
//
//    loader->RegisterTypeHandler<MPoint, GexMayaPlugin::MPointHandler>();
//
//    loader->RegisterTypeHandler<MItGeometry*, GexMayaPlugin::MItGeometryHandler>();
//
//    loader->RegisterTypeHandler<MMatrix, GexMayaPlugin::MMatrixHandler>();
//
//    loader->RegisterTypeHandler<MDataBlock*, GexMayaPlugin::MDataBlockHandler>();
//
//    loader->RegisterNode<GexMayaPlugin::DeformerGraphBuilder>("Maya/DeformerGraph");
//}


#endif //GEXMAYA_NODES_H
