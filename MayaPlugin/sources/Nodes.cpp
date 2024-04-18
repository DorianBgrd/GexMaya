#include "MayaPlugin/include/Nodes.h"

#include "maya/MDataBlock.h"
#include "maya/MMatrix.h"
#include "maya/MWeight.h"


void GexMaya::DeformerGraph::InitAttributes()
{
    auto* db = CreateAttribute<MDataBlock*>(
            "DataBlock", Gex::AttrValueType::Single,
            Gex::AttrType::Input);
    db->SetInternal(true);

    auto* it = CreateAttribute<MItGeometry*>(
            "GeomIt", Gex::AttrValueType::Single,
            Gex::AttrType::Input);
    it->SetInternal(true);

    auto* mx = CreateAttribute<MMatrix>(
            "Matrix", Gex::AttrValueType::Single,
            Gex::AttrType::Input);
    mx->SetInternal(true);

    auto* ix = CreateAttribute<int>(
            "OutGeomIndex", Gex::AttrValueType::Single,
            Gex::AttrType::Input);
    ix->SetInternal(true);
}


bool GexMaya::DeformerGraph::Evaluate(Gex::NodeAttributeData &ctx,
                                            Gex::GraphContext &graphContext,
                                            Gex::NodeProfiler &profiler)
{
    return true;
}


void GexMaya::GeomIter::InitAttributes()
{
    CreateAttribute<MItGeometry*>("ItGeom", Gex::AttrValueType::Single,
                                  Gex::AttrType::Input);

    TSys::Enum space;
    space.AddValue(0, "Object");
    space.AddValue(1, "World");

    auto* p = CreateAttribute<MPoint>("Position", Gex::AttrValueType::Single,
                                            Gex::AttrType::Input);
    p->SetInternal(true);
    p->SetExternal(false);

    auto* s = CreateAttributeFromValue("Space", std::make_any<TSys::Enum>(space),
            Gex::AttrValueType::Single, Gex::AttrType::Input);
    s->SetInternal(true);
    s->SetExternal(false);

    auto* n = CreateAttribute<MPoint>("Normale", Gex::AttrValueType::Single,
                                      Gex::AttrType::Input);
    n->SetInternal(true);
    n->SetExternal(false);

    auto* w = CreateAttribute<float>("Weight",  Gex::AttrValueType::Single,
                                     Gex::AttrType::Input);
    w->SetInternal(true);
    w->SetExternal(false);

    auto* m = CreateAttribute<MMatrix>("Matrix",  Gex::AttrValueType::Single,
                                            Gex::AttrType::Input);
    m->SetInternal(true);
    m->SetExternal(false);

    auto* op = CreateAttribute<MPoint>("OutPosition",  Gex::AttrValueType::Single,
                                            Gex::AttrType::Output);
    op->SetInternal(true);
    m->SetExternal(false);
}


bool GexMaya::GeomIter::PreEvaluate(Gex::NodeAttributeData &ctx,
                                          Gex::GraphContext &graphContext,
                                          Gex::NodeProfiler &profiler)
{
    return true;
}

bool GexMaya::GeomIter::Evaluate(Gex::NodeAttributeData &ctx,
                                       Gex::GraphContext &graphContext,
                                       Gex::NodeProfiler &profiler)
{
    auto iter = ctx.GetAttribute("ItGeom").GetValue<MItGeometry*>();

    Gex::NodeEvaluator evaluator(schelNodes, graphContext, profiler.GetProfiler());
    while (!iter->isDone())
    {
        MPoint pos;
        auto space = ctx.GetAttribute("Space").GetValue<TSys::Enum>();

        auto mspace = MSpace::kObject;
        if (space.CurrentIndex())
        {
            mspace = MSpace::kWorld;
        }

        ctx.GetAttribute("Position").SetValue(iter->position(mspace));
        ctx.GetAttribute("Weight").SetValue(iter->weight().influence());
        ctx.GetAttribute("Normale").SetValue(iter->normal(mspace));

        evaluator.Run();

        if (evaluator.Status() != Gex::NodeEvaluator::EvaluationStatus::Done)
        {
            return false;
        }

        PullInternalOutputs();

        MPoint op = ctx.GetAttribute("OutPosition").GetValue<MPoint>();
        iter->setPosition(op, mspace);

        iter->next();
    }

    return true;
}


bool GexMaya::GeomIter::PostEvaluate(Gex::NodeAttributeData &ctx,
                                           Gex::GraphContext &graphContext,
                                           Gex::NodeProfiler &profiler)
{
    return true;
}


Gex::ScheduleNodeList GexMaya::GeomIter::ToScheduledNodes()
{
    schelNodes = Gex::ScheduleNodes(GetNodes());

    return Gex::Node::ToScheduledNodes();
}



