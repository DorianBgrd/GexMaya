#ifndef GEXMAYA_DEFS_H
#define GEXMAYA_DEFS_H

#include "maya/MStatus.h"
#include "maya/MGlobal.h"


#define GEX_GRAPH_ATTR "gexGraph"
#define GEX_GRAPH_ATTR_SHORT "gxgrh"

#define GRAPH_INPUTS_ATTR "gexInputs"
#define GRAPH_INPUTS_ATTR_SHORT "gxi"

#define GRAPH_INPUTS_NAME_ATTR "gexInputName"
#define GRAPH_INPUTS_NAME_ATTR_SHORT "gxin"

#define GRAPH_INPUTS_VALUE_ATTR "gexInputValue"
#define GRAPH_INPUTS_VALUE_ATTR_SHORT "gxiv"

#define GRAPH_OUTPUTS_ATTR "gexOutputs"
#define GRAPH_OUTPUTS_ATTR_SHORT "gxo"

#define GRAPH_OUTPUTS_NAME_ATTR "gexOutputName"
#define GRAPH_OUTPUTS_NAME_ATTR_SHORT "gxon"

#define GRAPH_OUTPUTS_VALUE_ATTR "gexOutputValue"
#define GRAPH_OUTPUTS_VALUE_ATTR_SHORT "gxov"


#define GEX_NODE_ATTRIBUTES \
static MObject gexGraph; \
\
static MObject gexInputs;   \
static MObject gexInputName; \
static MObject gexInputValue; \
\
static MObject gexOutputs; \
static MObject gexOutputName; \
static MObject gexOutputValue;


#define GEX_NODE_ATTRIBUTES_DEF(class_) \
MObject class_::gexGraph; \
\
MObject class_::gexInputs; \
MObject class_::gexInputName; \
MObject class_::gexInputValue; \
\
MObject class_::gexOutputs; \
MObject class_::gexOutputName; \
MObject class_::gexOutputValue;


#define GEX_NODE_INITIALIZE() \
MFnTypedAttribute at;         \
\
gexGraph = at.create(GEX_GRAPH_ATTR, GEX_GRAPH_ATTR_SHORT, \
                     GexMaya::GraphData::id); \
at.setChannelBox(false); \
at.setKeyable(false); \
at.setHidden(true); \
\
addAttribute(gexGraph); \
\
MFnTypedAttribute typedAttr; \
gexInputName = typedAttr.create(GRAPH_INPUTS_NAME_ATTR, \
                                GRAPH_INPUTS_NAME_ATTR_SHORT, \
                                MFnData::kString); \
typedAttr.setArray(true); \
addAttribute(gexInputName); \
\
MFnGenericAttribute gen; \
gexInputValue = gen.create(GRAPH_INPUTS_VALUE_ATTR, GRAPH_INPUTS_VALUE_ATTR_SHORT); \
gen.addNumericDataAccept(MFnNumericData::kLong); \
gen.addNumericDataAccept(MFnNumericData::kBoolean); \
gen.addNumericDataAccept(MFnNumericData::kDouble); \
gen.addNumericDataAccept(MFnNumericData::kFloat); \
gen.addDataAccept(MFnData::kString); \
gen.addDataAccept(MFnData::kNurbsCurve); \
gen.addDataAccept(MFnData::kNurbsSurface); \
gen.addDataAccept(MFnData::kMesh); \
gen.addDataAccept(MFnData::kMatrix); \
gen.setArray(true); \
addAttribute(gexInputValue); \
\
gexOutputName = typedAttr.create(GRAPH_OUTPUTS_NAME_ATTR, \
                                 GRAPH_OUTPUTS_NAME_ATTR_SHORT, \
                                 MFnData::kString);        \
typedAttr.setStorable(true);  \
typedAttr.setArray(true);  \
addAttribute(gexOutputName); \
\
gexOutputValue = gen.create(GRAPH_OUTPUTS_VALUE_ATTR, GRAPH_OUTPUTS_VALUE_ATTR_SHORT); \
gen.setConnectable(true); \
gen.setKeyable(false); \
gen.setStorable(false); \
gen.addNumericDataAccept(MFnNumericData::kLong); \
gen.addNumericDataAccept(MFnNumericData::kBoolean); \
gen.addNumericDataAccept(MFnNumericData::kDouble); \
gen.addNumericDataAccept(MFnNumericData::kFloat); \
gen.addDataAccept(MFnData::kString); \
gen.addDataAccept(MFnData::kNurbsCurve); \
gen.addDataAccept(MFnData::kNurbsSurface); \
gen.addDataAccept(MFnData::kMesh); \
gen.addDataAccept(MFnData::kMatrix); \
gen.setArray(true); \
addAttribute(gexOutputValue); \
\
attributeAffects(gexInputValue, gexOutputValue);\
attributeAffects(gexInputName, gexOutputValue);\
attributeAffects(gexOutputName, gexOutputValue);


#endif //GEXMAYA_DEFS_H
