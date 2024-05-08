#ifndef GEXMAYA_DATA_H
#define GEXMAYA_DATA_H

#include "Gex/Gex/include/Gex.h"

#include "maya/MTypeId.h"
#include "maya/MStatus.h"
#include "maya/MPxData.h"

#include "api.h"
#include "defs.h"

#include <map>
#include <vector>


namespace GexMaya {
    typedef std::shared_ptr<Gex::CompoundNode> GraphPtr;

    class GEX_MAYA GraphData: public MPxData
    {
        GraphPtr g;

    public:
        GraphData();

        ~GraphData();

        GraphPtr SharedPtr() const;

        Gex::CompoundNode* Graph() const;

        void SetGraph(Gex::CompoundNode* node);

        static MTypeId id;

        MTypeId typeId() const override;

        MStatus readASCII(const MArgList &argList, unsigned int &endOfTheLastParsedElement) override;

        MStatus readBinary(std::istream &in, unsigned int length) override;

        MStatus writeASCII(std::ostream &out) override;

        MStatus writeBinary(std::ostream &out) override;

        void copy(const MPxData& src) override;

        MString name() const override;

        static void* create();

        static bool IsGraphData(MPxData* other);
    };

}

#endif //GEXMAYA_DATA_H
