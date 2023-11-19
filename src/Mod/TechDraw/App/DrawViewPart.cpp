/***************************************************************************
 *   Copyright (c) 2002 Jürgen Riegel <juergen.riegel@web.de>              *
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *   Copyright (c) 2016 WandererFan <wandererfan@gmail.com>                *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

//===========================================================================
// DrawViewPart overview
//===========================================================================
//
// 1) get the shapes from the source objects
// 2) center, scale and rotate the shapes
// 3) project the shape using the OCC HLR algorithms
// 4) add cosmetic and other objects that don't participate in hlr
// 5) find the closed regions (faces) in the edges returned by hlr
// everything else is mostly providing services to other objects, such as the
// actual drawing routines in Gui

#include "PreCompiled.h"

#include <thread>

#ifndef _PreComp_
#include <BRepAlgo_NormalProjection.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <HLRAlgo_Projector.hxx>
#include <QtConcurrentRun>
#include <ShapeAnalysis.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <sstream>
#endif

#include <App/Document.h>
#include <Base/BoundBox.h>
#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Parameter.h>

#include "Cosmetic.h"
#include "CenterLine.h"
#include "DrawGeomHatch.h"
#include "DrawHatch.h"
#include "DrawPage.h"
#include "DrawProjectSplit.h"
#include "DrawUtil.h"
#include "DrawViewBalloon.h"
#include "DrawViewDetail.h"
#include "DrawViewDimension.h"
#include "DrawViewPart.h"
#include "DrawViewPartPy.h"// generated from DrawViewPartPy.xml
#include "DrawViewSection.h"
#include "EdgeWalker.h"
#include "Geometry.h"
#include "GeometryObject.h"
#include "ShapeExtractor.h"
#include "Preferences.h"
#include "ShapeUtils.h"

using namespace TechDraw;
using DU = DrawUtil;

PROPERTY_SOURCE_WITH_EXTENSIONS(TechDraw::DrawViewPart, TechDraw::DrawView)

DrawViewPart::DrawViewPart(void)
    : concurrentData(this)
{
    static const char* group = "Projection";
    static const char* sgroup = "HLR Parameters";

    CosmeticExtension::initExtension(this);

    //properties that affect Geometry
    ADD_PROPERTY_TYPE(Source, (nullptr), group, App::Prop_None, "3D Shape to view");
    Source.setScope(App::LinkScope::Global);
    Source.setAllowExternal(true);
    ADD_PROPERTY_TYPE(XSource, (nullptr), group, App::Prop_None, "External 3D Shape to view");

    ADD_PROPERTY_TYPE(Direction, (0.0, -1.0, 0.0), group, App::Prop_None,
                      "Projection Plane normal. The direction you are looking from.");
    ADD_PROPERTY_TYPE(XDirection, (0.0, 0.0, 0.0), group, App::Prop_None,
                      "Projection Plane X Axis in R3. Rotates/Mirrors View");
    ADD_PROPERTY_TYPE(Perspective, (false), group, App::Prop_None,
                      "Perspective(true) or Orthographic(false) projection");
    ADD_PROPERTY_TYPE(Focus, (Preferences::getPreferenceGroup("General")->GetFloat("FocusDistance", 100.0)),
                    group, App::Prop_None, "Perspective view focus distance");

    //properties that control HLR algo
    ADD_PROPERTY_TYPE(CoarseView, (Preferences::getPreferenceGroup("General")->GetBool("CoarseView", false)),
        sgroup, App::Prop_None, "Coarse View on/off");
    ADD_PROPERTY_TYPE(SmoothVisible, (Preferences::getPreferenceGroup("HLR")->GetBool("SmoothViz", true)),
        sgroup, App::Prop_None, "Show Visible Smooth lines");
    ADD_PROPERTY_TYPE(SeamVisible, (Preferences::getPreferenceGroup("HLR")->GetBool("SeamViz", false)),
        sgroup, App::Prop_None,
                      "Show Visible Seam lines");
    ADD_PROPERTY_TYPE(IsoVisible, (Preferences::getPreferenceGroup("HLR")->GetBool("IsoViz", false)),
        sgroup, App::Prop_None, "Show Visible Iso u, v lines");
    ADD_PROPERTY_TYPE(HardHidden, (Preferences::getPreferenceGroup("HLR")->GetBool("HardHid", false)),
        sgroup, App::Prop_None, "Show Hidden Hard lines");
    ADD_PROPERTY_TYPE(SmoothHidden, (Preferences::getPreferenceGroup("HLR")->GetBool("SmoothHid", false)),
        sgroup, App::Prop_None, "Show Hidden Smooth lines");
    ADD_PROPERTY_TYPE(SeamHidden, (Preferences::getPreferenceGroup("HLR")->GetBool("SeamHid", false)),
        sgroup, App::Prop_None, "Show Hidden Seam lines");
    ADD_PROPERTY_TYPE(IsoHidden, (Preferences::getPreferenceGroup("HLR")->GetBool("IsoHid", false)),
        sgroup, App::Prop_None, "Show Hidden Iso u, v lines");
    ADD_PROPERTY_TYPE(IsoCount, (Preferences::getPreferenceGroup("HLR")->GetBool("IsoCount", 0)),
        sgroup, App::Prop_None, "Number of iso parameters lines");

    ADD_PROPERTY_TYPE(ScrubCount, (Preferences::scrubCount()), sgroup, App::Prop_None,
                      "The number of times FreeCAD should try to clean the HLR result.");
}

DrawViewPart::~DrawViewPart()
{
    removeAllReferencesFromGeom();
}

DrawViewPart::ConcurrentData::ConcurrentData(DrawViewPart* self)
    : geometryObject(self->getNameInDocument(), self)
{}


//! returns a compound of all the shapes from the DocumentObjects in the Source &
//!  XSource property lists
TopoDS_Shape DrawViewPart::getSourceShape(bool fuse) const
{
    const std::vector<App::DocumentObject*>& links = getAllSources();
    if (links.empty()) {
        return TopoDS_Shape();
    }
    if (fuse) {
        return ShapeExtractor::getShapesFused(links);
    }
    return ShapeExtractor::getShapes(links);
}

//! deliver a shape appropriate for making a detail view based on this view
//! TODO: why does dvp do the thinking for detail, but section picks its own
//! version of the shape?  Should we have a getShapeForSection?
TopoDS_Shape DrawViewPart::getShapeForDetail() const
{
    return ShapeUtils::rotateShape(getSourceShape(true), getProjectionCS(), Rotation.getValue());
}

//! combine the regular links and xlinks into a single list
std::vector<App::DocumentObject*> DrawViewPart::getAllSources() const
{
    //    Base::Console().Message("DVP::getAllSources()\n");
    std::vector<App::DocumentObject*> links = Source.getValues();
    std::vector<App::DocumentObject*> xLinks = XSource.getValues();

    std::vector<App::DocumentObject*> result = links;
    if (!xLinks.empty()) {
        result.insert(result.end(), xLinks.begin(), xLinks.end());
    }
    return result;
}

//! pick supported 2d shapes out of the Source properties and
//! add them directly to the geometry without going through HLR
//! NOTE: this is for loose 2d shapes such as Part line or circle and is
//! not meant to include complex 2d shapes such as Sketches.
void DrawViewPart::addShapes2d(void)
{
    auto lock = concurrentData.continueWriting();
    if(!lock) {
        return;
    }

    // get all the 2d shapes in the sources, then pick through them for loose edges
    // or vertices.
    std::vector<TopoDS_Shape> shapes = ShapeExtractor::getShapes2d(getAllSources());

    for (auto& s : shapes) {
        if (s.ShapeType() == TopAbs_VERTEX) {
            gp_Pnt gp = BRep_Tool::Pnt(TopoDS::Vertex(s));
            Base::Vector3d vp(gp.X(), gp.Y(), gp.Z());
            vp = vp - lock->centroid;
            //need to offset the point to match the big projection
            Base::Vector3d projected = projectPoint(vp * getScale());
            TechDraw::VertexPtr v1(std::make_shared<TechDraw::Vertex>(projected));

            lock->geometryObject.addVertex(v1);
        }
        else if (s.ShapeType() == TopAbs_EDGE) {
            Base::Console().Message("DVP::add2dShapes - found loose edge - isNull: %d\n", s.IsNull());
            TopoDS_Shape sTrans =
                ShapeUtils::moveShape(s, -lock->centroid);
            TopoDS_Shape sScale = ShapeUtils::scaleShape(sTrans,
                                                       getScale());
            TopoDS_Shape sMirror = ShapeUtils::mirrorShape(sScale);
            TopoDS_Edge edge = TopoDS::Edge(sMirror);
            BaseGeomPtr bg = projectEdge(edge);

            lock->geometryObject.addEdge(bg);
        }
    }
}

App::DocumentObjectExecReturn* DrawViewPart::execute(void)
{
    //    Base::Console().Message("DVP::execute() - %s\n", _getNameInDocument());
    if (!keepUpdated()) {
        return DrawView::execute();
    }

    TopoDS_Shape shape = getSourceShape();
    if (shape.IsNull()) {
        Base::Console().Message("DVP::execute - %s - Source shape is Null.\n", _getNameInDocument());
        return DrawView::execute();
    }

    //make sure the XDirection property is valid. Mostly for older models.
    if (!checkXDirection()) {
        Base::Vector3d newX = getXDirection();
        XDirection.setValue(newX);
        XDirection.purgeTouched();//don't trigger updates!
    }

    makeGeometryForShape(shape);

    return DrawView::execute();
}

short DrawViewPart::mustExecute() const
{
    if (isRestoring()) {
        return TechDraw::DrawView::mustExecute();
    }

    if (Direction.isTouched() || Source.isTouched() || XSource.isTouched()
        || Perspective.isTouched() || Focus.isTouched() || Rotation.isTouched()
        || SmoothVisible.isTouched() || SeamVisible.isTouched() || IsoVisible.isTouched()
        || HardHidden.isTouched() || SmoothHidden.isTouched() || SeamHidden.isTouched()
        || IsoHidden.isTouched() || IsoCount.isTouched() || CoarseView.isTouched()
        || CosmeticVertexes.isTouched() || CosmeticEdges.isTouched() || CenterLines.isTouched()) {
        return 1;
    }

    return TechDraw::DrawView::mustExecute();
}

void DrawViewPart::onChanged(const App::Property* prop)
{
    // If the user has set PropertyVector Direction to zero, set it along the default value instead (Front View).
    // Otherwise bad things will happen because there'll be a normalization for direction calculations later.
    Base::Vector3d dir = Direction.getValue();
    if (DrawUtil::fpCompare(dir.Length(), 0.0)) {
        Direction.setValue(Base::Vector3d(0.0, -1.0, 0.0));
    }

    DrawView::onChanged(prop);
}

//! prepare the shape for HLR processing by centering, scaling and rotating it
void DrawViewPart::makeGeometryForShape(const TopoDS_Shape& shape)
{
    auto lock = concurrentData.startWriting();
    gp_Pnt gCentroid = ShapeUtils::findCentroid(shape, getProjectionCS());
    auto centroid = DU::toVector3d(gCentroid);
    lock->centroid = centroid;
    lock.release();

    // if we use the passed reference directly, the centering doesn't work.  Maybe the underlying OCC TShape
    // isn't modified?  using a copy works and the referenced shape (from getSourceShape in execute())
    // isn't used for anything anyway.
    BRepBuilderAPI_Copy copier(shape);
    auto localShape = copier.Shape();

    localShape = centerScaleRotate(this, localShape, centroid);

    buildGeometryObject(std::move(localShape), getProjectionCS());
}

//! Modify a shape by centering, scaling and rotating and return the centered (but not rotated) shape
TopoDS_Shape DrawViewPart::centerScaleRotate(DrawViewPart* dvp, TopoDS_Shape& inOutShape,
                                             Base::Vector3d centroid)
{
    gp_Ax2 viewCS = dvp->getProjectionCS();

    //center shape on origin
    TopoDS_Shape centeredShape = ShapeUtils::moveShape(inOutShape, -centroid);

    inOutShape = ShapeUtils::scaleShape(centeredShape, dvp->getScale());
    if (!DrawUtil::fpCompare(dvp->Rotation.getValue(), 0.0)) {
        inOutShape = ShapeUtils::rotateShape(inOutShape, viewCS,
                                           dvp->Rotation.getValue());//conventional rotation
    }
    //    BRepTools::Write(inOutShape, "DVPScaled.brep");            //debug
    return centeredShape;
}

void DrawViewPart::buildGeometryObject(const TopoDS_Shape& shape, const gp_Ax2& viewCS)
{
    auto lock = concurrentData.startWriting();
    lock.release();
    BRepBuilderAPI_Copy copier(shape);

    buildGeometryObject(copier.Shape(), viewCS);
}

void DrawViewPart::buildGeometryObject(TopoDS_Shape&& shape, const gp_Ax2& viewCS)
{
    showProgressMessage(getNameInDocument(), "is finding hidden lines");

    auto lock = concurrentData.continueWriting();
    if(!lock) {
        return;
    }

    lock->geometryObject.setIsoCount(IsoCount.getValue());
    lock->geometryObject.isPerspective(Perspective.getValue());
    lock->geometryObject.setFocus(Focus.getValue());
    lock->geometryObject.usePolygonHLR(CoarseView.getValue());
    lock->geometryObject.setScrubCount(ScrubCount.getValue());

    lock.moveFromThread();

    // Although the "self" variable is not being accessed,
    // it warrants that "this" will not be destructed meanwhile.
    auto lambda = [self = SharedFromThis(), this, lock = std::move(lock),
                   shape = std::move(shape), viewCS]() mutable noexcept
    {
        try{
            lock.resumeInNewThread();
            GeometryObject go{lock->geometryObject};
            lock.release();

                    //the polygon approximation HLR process runs quickly
            go.projectShapeWithPolygonAlgo(shape, viewCS);

            if (!CoarseView.getValue()) {
                go.projectShape(shape, viewCS);
            }

            if(!lock.resume()) {
                return;
            }
            lock->geometryObject = go;
            lock.release();

            onHlrFinished();
            if(!lock) {
                return;
            }

            extractFaces();
            if(!lock) {
                return;
            }

            onFacesFinished();
        }
        catch(...) {
            Base::Console().Message("DVP::buildGeometryObject - failed to build geometry object");
        }
    };

    std::thread{std::move(lambda)}.detach();
//    std::thread{std::move(lambda)}.join();
}

//! continue processing after hlr thread completes
void DrawViewPart::onHlrFinished()
{
    auto lock = concurrentData.continueWriting();
    if(!lock) {
        return;
    }
    lock->bbox = lock->geometryObject.calcBoundingBox();
    lock.release();

    showProgressMessage(getNameInDocument(), "has finished finding hidden lines");

    //application level tasks that depend on HLR/GO being complete
    postHlrTasks();
}

//! run any tasks that need to been done after geometry is available
void DrawViewPart::postHlrTasks()
{
    //add geometry that doesn't come from HLR
    addCosmeticVertexesToGeom();
    addCosmeticEdgesToGeom();
    addReferencesToGeom();
    addShapes2d();

    //balloons need to be recomputed here because their
    //references will be invalid until the geometry exists
    for (auto& b: getBalloons()) {
        b->recomputeFeature();
    }
    // Dimensions need to be recomputed now if face finding is not going to take place.
    if (!handleFaces() || CoarseView.getValue()) {
        auto dims = getDimensions();
        for (auto& d : dims) {
            d->recomputeFeature();
        }
    }

    //second pass if required
    if (ScaleType.isValue("Automatic") && !checkFit()) {
        double newScale = autoScale();
        Scale.setValue(newScale);
        Scale.purgeTouched();
        // TODO: we should mark for update, not call recursivelly!
        // If anyone answers...
        // https://forum.freecad.org/viewtopic.php?t=82503
        // partExec(m_saveShape);
    }

    // TODO: investigate/eliminate this!!!
    // Sounds like a very complicated way to avoid infinite recursions.
    overrideKeepUpdated(false);

    requestPaint();
}

// Run any tasks that need to be done after faces are available
void DrawViewPart::postFaceExtractionTasks()
{
    auto lock = concurrentData.continueWriting();
    if(!lock) {
        return;
    }
    // Some centerlines depend on faces so we could not add CL geometry before now
    addCenterLinesToGeom();
    lock.release();

    // Dimensions need to be recomputed because their references will be invalid
    //  until all the geometry (including centerlines dependent on faces) exists.
    auto dims = getDimensions();
    for (auto& d : dims) {
        d->recomputeFeature();
    }

    requestPaint();
}


//! make faces from the edge geometry
void DrawViewPart::extractFaces()
{
    if (!handleFaces()) {
        return;
    }
    if (CoarseView.getValue()) {
        return;
    }

    showProgressMessage(getNameInDocument(), "is extracting faces");

    auto lock = concurrentData.continueReading();
    if(!lock) {
        return;
    }
    const std::vector<BaseGeomPtr>& goEdges =
        lock->geometryObject.getVisibleFaceEdges(SmoothVisible.getValue(),
                                                 SeamVisible.getValue());
    lock.release();

    if (goEdges.empty()) {
        return;
    }

    if (newFaceFinder()) {
        findFacesNew(goEdges);
    } else {
        findFacesOld(goEdges);
    }
}

// use the revised face finder algo
void DrawViewPart::findFacesNew(const std::vector<BaseGeomPtr> &goEdges)
{
    std::vector<TopoDS_Edge> closedEdges;
    std::vector<TopoDS_Edge> cleanEdges = DrawProjectSplit::scrubEdges(goEdges, closedEdges);

    if (cleanEdges.empty() && closedEdges.empty()) {
        //how does this happen?  something wrong somewhere
        //            Base::Console().Message("DVP::findFacesNew - no clean or closed wires\n");    //debug
        return;
    }

    //use EdgeWalker to make wires from edges
    EdgeWalker eWalker;
    std::vector<TopoDS_Wire> sortedWires;
    try {
        if (!cleanEdges.empty()) {
            sortedWires = eWalker.execute(cleanEdges, true);//include outer wire
        }
    }
    catch (Base::Exception& e) {
        throw Base::RuntimeError(e.what());
    }

    std::vector<TopoDS_Wire> closedWires;
    for (auto& e : closedEdges) {
        BRepBuilderAPI_MakeWire mkWire(e);
        TopoDS_Wire w = mkWire.Wire();
        closedWires.push_back(w);
    }
    if (!closedWires.empty()) {
        sortedWires.insert(sortedWires.end(), closedWires.begin(), closedWires.end());
        //inserting the closedWires that did not go through EdgeWalker into
        //sortedWires ruins EdgeWalker's sort by size, so we have to do it again.
        sortedWires = eWalker.sortWiresBySize(sortedWires);
    }

    auto lock = concurrentData.continueWriting();
    if(!lock) {
        return;
    }
    lock->geometryObject.clearFaceGeom();

    if (sortedWires.empty()) {
        Base::Console().Warning(
            "DVP::findFacesNew - %s - Can't make faces from projected edges\n",
            _getNameInDocument());
    }
    else {
        constexpr double minWireArea = 0.000001;//arbitrary very small face size
        for (const auto& wire: sortedWires) {
            if (!BRep_Tool::IsClosed(wire)) {
                continue;//can not make a face from open wire
            }

            double area = ShapeAnalysis::ContourArea(wire);
            if (area <= minWireArea) {
                continue;//can not make a face from wire with no area
            }

            FacePtr f(std::make_shared<Face>());
            f->wires.push_back(new Wire(wire));
            lock->geometryObject.addFaceGeom(f);
        }
    }
}

// original face finding method
void DrawViewPart::findFacesOld(const std::vector<BaseGeomPtr> &goEdges)
{
    //make a copy of the input edges so the loose tolerances of face finding are
    //not applied to the real edge geometry.  See TopoDS_Shape::TShape().
    std::vector<TopoDS_Edge> copyEdges;
    for (const auto& e : goEdges) {
        BRepBuilderAPI_Copy copier(e->getOCCEdge());
        copyEdges.push_back(TopoDS::Edge(copier.Shape()));
    }
    std::vector<TopoDS_Edge> nonZero;
    for (auto& e : copyEdges) {//drop any zero edges (shouldn't be any by now!!!)
        if (!DrawUtil::isZeroEdge(e)) {
            nonZero.push_back(e);
        }
    }

    //HLR algo does not provide all edge intersections for edge endpoints.
    //need to split long edges touched by Vertex of another edge
    std::vector<splitPoint> splits;
    for (auto& outer: nonZero) {
        TopoDS_Vertex v1 = TopExp::FirstVertex(outer);
        TopoDS_Vertex v2 = TopExp::LastVertex(outer);
        Bnd_Box sOuter;
        BRepBndLib::AddOptimal(outer, sOuter);
        sOuter.SetGap(0.1);
        if (sOuter.IsVoid()) {
            continue;
        }
        if (DrawUtil::isZeroEdge(outer)) {
            continue;                   //skip zero length edges. shouldn't happen ;)
        }
        int innerCounter = -1; // TODO: get rid of this!
        for (auto& inner: nonZero) {
            ++innerCounter;
            if (&outer == &inner) {
                continue;
            }
            if (DrawUtil::isZeroEdge(inner)) {
                continue;//skip zero length edges. shouldn't happen ;)
            }

            Bnd_Box sInner;
            BRepBndLib::AddOptimal(inner, sInner);
            sInner.SetGap(0.1);
            if (sInner.IsVoid()) {
                continue;
            }
            if (sOuter.IsOut(sInner)) {//bboxes of edges don't intersect, don't bother
                continue;
            }

            double param = -1;
            if (DrawProjectSplit::isOnEdge(inner, v1, param, false)) {
                gp_Pnt pnt1 = BRep_Tool::Pnt(v1);
                splitPoint s1;
                s1.i = innerCounter;
                s1.v = Base::Vector3d(pnt1.X(), pnt1.Y(), pnt1.Z());
                s1.param = param;
                splits.push_back(s1);
            }
            if (DrawProjectSplit::isOnEdge(inner, v2, param, false)) {
                gp_Pnt pnt2 = BRep_Tool::Pnt(v2);
                splitPoint s2;
                s2.i = innerCounter;
                s2.v = Base::Vector3d(pnt2.X(), pnt2.Y(), pnt2.Z());
                s2.param = param;
                splits.push_back(s2);
            }
        }//inner loop
    }    //outer loop

    std::vector<splitPoint> sorted = DrawProjectSplit::sortSplits(splits, true);
    auto last = std::unique(sorted.begin(), sorted.end(),
                            DrawProjectSplit::splitEqual);//duplicates to back
    sorted.erase(last, sorted.end());                     //remove dupl splits
    std::vector<TopoDS_Edge> newEdges = DrawProjectSplit::splitEdges(nonZero, sorted);

    if (newEdges.empty()) {
        return;
    }

    newEdges = DrawProjectSplit::removeDuplicateEdges(newEdges);

    //find all the wires in the pile of faceEdges
    std::vector<TopoDS_Wire> sortedWires;
    EdgeWalker eWalker;
    sortedWires = eWalker.execute(newEdges);

    auto lock = concurrentData.continueWriting();
    if(!lock) {
        return;
    }
    if (sortedWires.empty()) {
        lock->geometryObject.clearFaceGeom();
        lock.release();
        Base::Console().Warning(
            "DVP::findFacesOld - %s -Can't make faces from projected edges\n",
            _getNameInDocument());
        return;
    }

    lock->geometryObject.clearFaceGeom();
    for (const auto& wire: sortedWires) {
        //version 1: 1 wire/face - no voids in face
        FacePtr f(std::make_shared<Face>());
        TechDraw::Wire* w = new TechDraw::Wire(wire);
        f->wires.push_back(w);
        lock->geometryObject.addFaceGeom(f);
    }
}

//continue processing after extractFaces thread completes
void DrawViewPart::onFacesFinished()
{
    showProgressMessage(getNameInDocument(), "has finished extracting faces");

    // Now we can recompute Dimensions and do other tasks possibly depending on Face extraction
    postFaceExtractionTasks();

    requestPaint();
}

//retrieve all the face hatches associated with this dvp
std::vector<TechDraw::DrawHatch*> DrawViewPart::getHatches() const
{
    std::vector<TechDraw::DrawHatch*> result;
    std::vector<App::DocumentObject*> children = getInList();
    for (auto& child : children) {
        if (child->isDerivedFrom<DrawHatch>() && !child->isRemoving()) {
            TechDraw::DrawHatch* hatch = dynamic_cast<TechDraw::DrawHatch*>(child);
            result.push_back(hatch);
        }
    }
    return result;
}

//retrieve all the geometric hatches associated with this dvp
std::vector<TechDraw::DrawGeomHatch*> DrawViewPart::getGeomHatches() const
{
    std::vector<TechDraw::DrawGeomHatch*> result;
    std::vector<App::DocumentObject*> children = getInList();
    for (auto& child : children) {
        if (child->isDerivedFrom<DrawGeomHatch>()
            && !child->isRemoving()) {
            TechDraw::DrawGeomHatch* geom = dynamic_cast<TechDraw::DrawGeomHatch*>(child);
            result.push_back(geom);
        }
    }
    return result;
}

//return *unique* list of Dimensions which reference this DVP
//if the dimension has two references to this dvp, it will appear twice in
//the inlist
std::vector<std::shared_ptr<DrawViewDimension>> DrawViewPart::getDimensions() const
{
    std::vector<std::shared_ptr<DrawViewDimension>> result;
    auto children = getInListNew();
    std::sort(children.begin(), children.end());
    // TODO: is it not unique? if it is not... shouldn't it be?
    auto newEnd = std::unique(children.begin(), children.end());
    for (auto it = children.begin(); it != newEnd; ++it) {
        if ((*it)->isDerivedFrom<DrawViewDimension>()) {
            auto dim = std::dynamic_pointer_cast<DrawViewDimension>(std::move(*it));
            result.emplace_back(std::move(dim));
        }
    }
    return result;
}

std::vector<std::shared_ptr<DrawViewBalloon>> DrawViewPart::getBalloons() const
{
    std::vector<std::shared_ptr<DrawViewBalloon>> result;
    auto children = getInListNew();
    std::sort(children.begin(), children.end());
    auto newEnd = std::unique(children.begin(), children.end());
    for (auto it = children.begin(); it != newEnd; ++it) {
        if ((*it)->isDerivedFrom<DrawViewBalloon>()) {
            auto balloon = std::dynamic_pointer_cast<DrawViewBalloon>(std::move(*it));
            result.emplace_back(std::move(balloon));
        }
    }
    return result;
}

const std::vector<VertexPtr> DrawViewPart::getVertexGeometry() const
{
    auto lock = concurrentData.lockForReading();
    return lock->geometryObject.getVertexGeometry();
}


//! TechDraw vertex names run from 0 to n-1
TechDraw::VertexPtr DrawViewPart::getVertex(std::string vertexName) const
{
    const std::vector<TechDraw::VertexPtr> allVertex(DrawViewPart::getVertexGeometry());
    size_t iTarget = DrawUtil::getIndexFromName(vertexName);
    if (allVertex.empty()) {
        //should not happen
        throw Base::IndexError("DVP::getVertex - No vertices found.");
    }
    if (iTarget >= allVertex.size()) {
        //should not happen
        throw Base::IndexError("DVP::getVertex - Vertex not found.");
    }

    return allVertex.at(iTarget);
}

//! returns existing BaseGeom of 2D Edge
//! TechDraw edge names run from 0 to n-1
TechDraw::BaseGeomPtr DrawViewPart::getEdge(std::string edgeName) const
{
    const std::vector<TechDraw::BaseGeomPtr>& geoms = getEdgeGeometry();
    if (geoms.empty()) {
        //should not happen
        throw Base::IndexError("DVP::getEdge - No edges found.");
    }
    size_t iEdge = DrawUtil::getIndexFromName(edgeName);
    if ((unsigned)iEdge >= geoms.size()) {
        throw Base::IndexError("DVP::getEdge - Edge not found.");
    }
    return geoms.at(iEdge);
}


//! returns existing 2d Face
//! TechDraw face names run from 0 to n-1
TechDraw::FacePtr DrawViewPart::getFace(std::string faceName) const
{
    const std::vector<TechDraw::FacePtr>& faces = getFaceGeometry();
    if (faces.empty()) {
        //should not happen
        throw Base::IndexError("DVP::getFace - No faces found.");
    }
    size_t iFace = DrawUtil::getIndexFromName(faceName);
    if (iFace >= faces.size()) {
        throw Base::IndexError("DVP::getFace - Face not found.");
    }
    return faces.at(iFace);
}


const std::vector<TechDraw::FacePtr> DrawViewPart::getFaceGeometry() const
{
    auto lock = concurrentData.lockForReading();
    return lock->geometryObject.getFaceGeometry();
}

const BaseGeomPtrVector DrawViewPart::getEdgeGeometry() const
{
    auto lock = concurrentData.lockForReading();
    return lock->geometryObject.getEdgeGeometry();
}

//! returns existing BaseGeom of 2D Edge(idx)
TechDraw::BaseGeomPtr DrawViewPart::getGeomByIndex(int idx) const
{
    const std::vector<TechDraw::BaseGeomPtr>& geoms = getEdgeGeometry();
    if (geoms.empty()) {
        return nullptr;
    }
    if ((unsigned)idx >= geoms.size()) {
        Base::Console().Error("DVP::getGeomByIndex(%d) - invalid index - size: %d\n", idx,
                              geoms.size());
        return nullptr;
    }
    return geoms.at(idx);
}

//! returns existing geometry of 2D Vertex(idx)
TechDraw::VertexPtr DrawViewPart::getProjVertexByIndex(int idx) const
{
    const std::vector<TechDraw::VertexPtr>& geoms = getVertexGeometry();
    if (geoms.empty()) {
        return nullptr;
    }
    if ((unsigned)idx >= geoms.size()) {
        Base::Console().Error("DVP::getProjVertexByIndex(%d) - invalid index\n", idx);
        return nullptr;
    }
    return geoms.at(idx);
}

TechDraw::VertexPtr DrawViewPart::getProjVertexByCosTag(std::string cosTag)
{
    std::vector<TechDraw::VertexPtr> gVerts = getVertexGeometry();
    if (gVerts.empty()) {
        return nullptr;
    }

    for (auto& gv : gVerts) {
        if (gv->getCosmeticTag() == cosTag) {
            return gv;
        }
    }
    return nullptr;
}


//! returns existing geometry of 2D Face(idx)
std::vector<TechDraw::BaseGeomPtr> DrawViewPart::getFaceEdgesByIndex(int idx) const
{
    std::vector<TechDraw::BaseGeomPtr> result;
    const std::vector<TechDraw::FacePtr>& faces = getFaceGeometry();
    if (idx < (int)faces.size()) {
        TechDraw::FacePtr projFace = faces.at(idx);
        for (auto& w : projFace->wires) {
            for (auto& g : w->geoms) {
                if (g->getCosmetic()) {
                    //if g is cosmetic, we should skip it
                    continue;
                }
                else {
                    result.push_back(g);
                }
            }
        }
    }
    return result;
}

std::vector<TopoDS_Wire> DrawViewPart::getWireForFace(int idx) const
{
    std::vector<TopoDS_Wire> result;
    std::vector<TopoDS_Edge> edges;
    const std::vector<TechDraw::FacePtr>& faces = getFaceGeometry();
    TechDraw::FacePtr ourFace = faces.at(idx);
    for (auto& w : ourFace->wires) {
        edges.clear();
        for (auto& g : w->geoms) {
            edges.push_back(g->getOCCEdge());
        }
        TopoDS_Wire occwire = EdgeWalker::makeCleanWire(edges);
        result.push_back(occwire);
    }

    return result;
}

Base::BoundBox3d DrawViewPart::getBoundingBox() const
{
    auto lock = concurrentData.lockForReading();
    return lock->bbox;
}

double DrawViewPart::getBoxX() const
{
    Base::BoundBox3d bbx = getBoundingBox();//bbox is already scaled & centered!
    return (bbx.MaxX - bbx.MinX);
}

double DrawViewPart::getBoxY() const
{
    Base::BoundBox3d bbx = getBoundingBox();
    return (bbx.MaxY - bbx.MinY);
}

QRectF DrawViewPart::getRect() const
{
    //    Base::Console().Message("DVP::getRect() - %s\n", _getNameInDocument());
    double x = getBoxX();
    double y = getBoxY();
    return QRectF(0.0, 0.0, x, y);
}

//returns a compound of all the visible projected edges
TopoDS_Shape DrawViewPart::getEdgeCompound() const
{
    auto lock = concurrentData.lockForReading();

    BRep_Builder builder;
    TopoDS_Compound result;
    builder.MakeCompound(result);

    auto hard = lock->geometryObject.getVisHard();
    if (!hard.IsNull()) {
        builder.Add(result, hard);
    }

    auto outline = lock->geometryObject.getVisOutline();
    if (!outline.IsNull()) {
        builder.Add(result, outline);
    }

    auto seam = lock->geometryObject.getVisSeam();
    if (!seam.IsNull()) {
        builder.Add(result, seam);
    }

    auto smooth = lock->geometryObject.getVisSmooth();
    if (!smooth.IsNull()) {
        builder.Add(result, smooth);
    }

    //check for empty compound
    if (result.IsNull() || !TopoDS_Iterator(result).More()) {
        return TopoDS_Shape();
    }
    return result;
}

// returns the (unscaled) size of the visible lines along the alignment vector.
// alignment vector is already projected onto our CS, so only has X,Y components
// used in calculating the length of a section line
double DrawViewPart::getSizeAlongVector(Base::Vector3d alignmentVector)
{
    //    Base::Console().Message("DVP::GetSizeAlongVector(%s)\n", DrawUtil::formatVector(alignmentVector).c_str());
    double alignmentAngle = -atan2(alignmentVector.y, alignmentVector.x);
    gp_Ax2 OXYZ;//shape has already been projected and we will rotate around Z
    if (getEdgeCompound().IsNull()) {
        return 1.0;
    }
    TopoDS_Shape rotatedShape = ShapeUtils::rotateShape(getEdgeCompound(), OXYZ, alignmentAngle * 180.0 / M_PI);
    Bnd_Box shapeBox;
    shapeBox.SetGap(0.0);
    BRepBndLib::AddOptimal(rotatedShape, shapeBox);
    double xMin = 0, xMax = 0, yMin = 0, yMax = 0, zMin = 0, zMax = 0;
    shapeBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    double shapeWidth((xMax - xMin) / getScale());
    return shapeWidth;
}

//used to project a pt (ex SectionOrigin) onto paper plane
Base::Vector3d DrawViewPart::projectPoint(const Base::Vector3d& pt, bool invert) const
{
    //    Base::Console().Message("DVP::projectPoint(%s, %d\n",
    //                            DrawUtil::formatVector(pt).c_str(), invert);
    Base::Vector3d stdOrg(0.0, 0.0, 0.0);
    gp_Ax2 viewCS = getProjectionCS(stdOrg);
    gp_Pnt gPt(pt.x, pt.y, pt.z);

    HLRAlgo_Projector projector(viewCS);
    gp_Pnt2d prjPnt;
    projector.Project(gPt, prjPnt);
    Base::Vector3d result(prjPnt.X(), prjPnt.Y(), 0.0);
    if (invert) {
        result = DrawUtil::invertY(result);
    }
    return result;
}

//project an edge onto the paper plane
BaseGeomPtr DrawViewPart::projectEdge(const TopoDS_Edge& e) const
{
    Base::Vector3d stdOrg(0.0, 0.0, 0.0);
    gp_Ax2 viewCS = getProjectionCS(stdOrg);

    gp_Pln plane(viewCS);
    TopoDS_Face paper = BRepBuilderAPI_MakeFace(plane);
    BRepAlgo_NormalProjection projector(paper);
    projector.Add(e);
    projector.Build();
    TopoDS_Shape s = projector.Projection();
    return BaseGeom::baseFactory(TopoDS::Edge(s));
}

bool DrawViewPart::hasGeometry(void) const
{
    auto lock = concurrentData.lockForReading();
    return (!getVertexGeometry().empty() || !getEdgeGeometry().empty());
}

//convert a vector in local XY coords into a coordinate system in global
//coordinates aligned to the vector.
//Note that this CS may not have the ideal XDirection for the derived view
//(likely a DrawViewSection) and the user may need to adjust the XDirection
//in the derived view.
gp_Ax2 DrawViewPart::localVectorToCS(const Base::Vector3d localUnit) const
{
    //    Base::Console().Message("DVP::localVectorToCS(%s)\n", DU::formatVector((localUnit)).c_str());
    double angle = atan2(localUnit.y, localUnit.x);//radians
    gp_Ax1 rotateAxisDir(gp_Pnt(0.0, 0.0, 0.0), getProjectionCS().Direction());
    gp_Vec gOldX = getProjectionCS().XDirection();
    gp_Vec gNewDirection = gOldX.Rotated(rotateAxisDir, angle);
    gp_Vec gNewY = getProjectionCS().Direction();
    gp_Vec gNewX = (gNewDirection.Crossed(gNewY).Reversed());
    if (gNewX.IsParallel(gOldX, EWTOLERANCE)) {
        //if the X directions are parallel, the view is rotating around X, so
        //we should use the original X to prevent unwanted mirroring.
        //There might be a better choice of tolerance than EWTOLERANCE
        gNewX = gOldX;
    }

    return {gp_Pnt(0.0, 0.0, 0.0), gp_Dir(gNewDirection), gp_Dir(gNewX)};
}


Base::Vector3d DrawViewPart::localVectorToDirection(const Base::Vector3d localUnit) const
{
    //    Base::Console().Message("DVP::localVectorToDirection() - localUnit: %s\n", DrawUtil::formatVector(localUnit).c_str());
    gp_Ax2 cs = localVectorToCS(localUnit);
    return DrawUtil::toVector3d(cs.Direction());
}

gp_Ax2 DrawViewPart::getProjectionCS(const Base::Vector3d pt) const
{
    //    Base::Console().Message("DVP::getProjectionCS() - %s - %s\n", _getNameInDocument(), Label.getValue());
    Base::Vector3d direction = Direction.getValue();
    gp_Dir gDir(direction.x, direction.y, direction.z);
    Base::Vector3d xDir = getXDirection();
    gp_Dir gXDir(xDir.x, xDir.y, xDir.z);
    gp_Pnt gOrg(pt.x, pt.y, pt.z);
    gp_Ax2 viewCS(gOrg, gDir);
    try {
        viewCS = gp_Ax2(gOrg, gDir, gXDir);
    }
    catch (...) {
        Base::Console().Warning("DVP - %s - failed to create projection CS\n", _getNameInDocument());
    }
    return viewCS;
}

gp_Ax2 DrawViewPart::getRotatedCS(const Base::Vector3d basePoint) const
{
    //    Base::Console().Message("DVP::getRotatedCS() - %s - %s\n", _getNameInDocument(), Label.getValue());
    gp_Ax2 unrotated = getProjectionCS(basePoint);
    gp_Ax1 rotationAxis(DU::togp_Pnt(basePoint), unrotated.Direction());
    double angleRad = Rotation.getValue() * M_PI / 180.0;
    gp_Ax2 rotated = unrotated.Rotated(rotationAxis, -angleRad);
    return rotated;
}

gp_Ax2 DrawViewPart::getViewCS(const Base::Vector3d& pt, const Base::Vector3d& /*direction*/,
                               const bool /*flip*/) const
{
    Base::Console().Message("DVP::getviewCS - deprecated. Use getProjectionCS.\n");
    return getProjectionCS(pt);
}

//TODO: make saveShape a property

Base::Vector3d DrawViewPart::getOriginalCentroid() const
{
    auto lock = concurrentData.lockForReading();
    return lock->centroid;
}

Base::Vector3d DrawViewPart::getCurrentCentroid() const
{
    TopoDS_Shape shape = getSourceShape();
    if (shape.IsNull()) {
        return Base::Vector3d(0.0, 0.0, 0.0);
    }
    gp_Ax2 cs = getProjectionCS();
    gp_Pnt gCenter = ShapeUtils::findCentroid(shape, cs);
    return DU::toVector3d(gCenter);
}

std::vector<DrawViewSection*> DrawViewPart::getSectionRefs() const
{
    std::vector<DrawViewSection*> result;
    std::vector<App::DocumentObject*> inObjs = getInList();
    for (auto& o : inObjs) {
        if (o->isDerivedFrom<DrawViewSection>()) {
            result.push_back(static_cast<TechDraw::DrawViewSection*>(o));
        }
    }
    return result;
}

std::vector<DrawViewDetail*> DrawViewPart::getDetailRefs() const
{
    std::vector<DrawViewDetail*> result;
    std::vector<App::DocumentObject*> inObjs = getInList();
    for (auto& o : inObjs) {
        if (o->isDerivedFrom<DrawViewDetail>()) {
            if (!o->isRemoving()) {
                result.push_back(static_cast<TechDraw::DrawViewDetail*>(o));
            }
        }
    }
    return result;
}

const BaseGeomPtrVector DrawViewPart::getVisibleFaceEdges() const
{
    auto lock = concurrentData.lockForReading();
    return lock->geometryObject.getVisibleFaceEdges(SmoothVisible.getValue(), SeamVisible.getValue());
}

bool DrawViewPart::handleFaces()
{
    return Preferences::getPreferenceGroup("General")->GetBool("HandleFaces", true);
}

bool DrawViewPart::newFaceFinder(void)
{
    return Preferences::getPreferenceGroup("General")->GetBool("NewFaceFinder", false);
}

//! remove features that are useless without this DVP
//! hatches, geomhatches, dimensions, ...
void DrawViewPart::unsetupObject()
{
//    Base::Console().Message("DVP::unsetupObject()\n");
    App::Document* doc = getDocument();
    std::string docName = doc->getName();

    // Remove the View's Hatches from document
    std::vector<TechDraw::DrawHatch*> hatches = getHatches();
    std::vector<TechDraw::DrawHatch*>::iterator it = hatches.begin();
    for (; it != hatches.end(); it++) {
        std::string viewName = (*it)->_getNameInDocument();
        Base::Interpreter().runStringArg("App.getDocument(\"%s\").removeObject(\"%s\")",
                                         docName.c_str(), viewName.c_str());
    }

    // Remove the View's GeomHatches from document
    std::vector<TechDraw::DrawGeomHatch*> gHatches = getGeomHatches();
    std::vector<TechDraw::DrawGeomHatch*>::iterator it2 = gHatches.begin();
    for (; it2 != gHatches.end(); it2++) {
        std::string viewName = (*it2)->_getNameInDocument();
        Base::Interpreter().runStringArg("App.getDocument(\"%s\").removeObject(\"%s\")",
                                         docName.c_str(), viewName.c_str());
    }

    // Remove Dimensions which reference this DVP
    // must use page->removeObject first
    TechDraw::DrawPage* page = findParentPage();
    if (page) {
        auto dims = getDimensions();
        for(auto& dim: dims) {
            page->removeView(dim.get());
            if (dim->isAttachedToDocument()) {
                Base::Interpreter().runStringArg("App.getDocument(\"%s\").removeObject(\"%s\")",
                                                 docName.c_str(), dim->_getNameInDocument());
            }
        }
    }

    // Remove Balloons which reference this DVP
    // must use page->removeObject first
    page = findParentPage();
    if (page) {
        for (auto& ballon: getBalloons()) {
            page->removeView(ballon.get());
            if (ballon->isAttachedToDocument()) {
                Base::Interpreter().runStringArg("App.getDocument(\"%s\").removeObject(\"%s\")",
                                                 docName.c_str(), ballon->_getNameInDocument());
            }
        }
    }
}

bool DrawViewPart::checkXDirection() const
{
    //    Base::Console().Message("DVP::checkXDirection()\n");
    Base::Vector3d xDir = XDirection.getValue();
    if (DrawUtil::fpCompare(xDir.Length(), 0.0)) {
        return false;
    }
    return true;
}

Base::Vector3d DrawViewPart::getXDirection() const
{
    //    Base::Console().Message("DVP::getXDirection() - %s\n", Label.getValue());
    Base::Vector3d result(1.0, 0.0, 0.0);//default X
    App::Property* prop = getPropertyByName("XDirection");
    if (prop) {//have an XDirection property
        Base::Vector3d propVal = XDirection.getValue();
        if (DrawUtil::fpCompare(propVal.Length(), 0.0)) {//but it has no value
            Base::Vector3d dir = Direction.getValue();   //make a sensible default
            Base::Vector3d org(0.0, 0.0, 0.0);
            result = getLegacyX(org, dir);
        } else {
            result = propVal;	//normal case.  XDirection is set.
        }
    }
    else {                                        //no Property.  can this happen?
        Base::Vector3d dir = Direction.getValue();//make a sensible default
        Base::Vector3d org(0.0, 0.0, 0.0);
        result = getLegacyX(org, dir);
    }
    return result;
}

Base::Vector3d DrawViewPart::getLegacyX(const Base::Vector3d& pt, const Base::Vector3d& axis,
                                        const bool flip) const
{
    //    Base::Console().Message("DVP::getLegacyX() - %s\n", Label.getValue());
    gp_Ax2 viewCS = ShapeUtils::legacyViewAxis1(pt, axis, flip);
    gp_Dir gXDir = viewCS.XDirection();
    return Base::Vector3d(gXDir.X(), gXDir.Y(), gXDir.Z());
}

// reference dimensions & their vertices
// these routines may be obsolete

void DrawViewPart::updateReferenceVert(std::string tag, Base::Vector3d loc2d)
{
    for (auto& v : m_referenceVerts) {
        if (v->getTagAsString() == tag) {
            v->point(loc2d);
            break;
        }
    }
}

void DrawViewPart::addReferencesToGeom()
{
    //    Base::Console().Message("DVP::addReferencesToGeom() - %s\n", _getNameInDocument());
    std::vector<TechDraw::VertexPtr> gVerts = getVertexGeometry();
    gVerts.insert(gVerts.end(), m_referenceVerts.begin(), m_referenceVerts.end());

    auto lock = concurrentData.continueWriting();
    lock->geometryObject.setVertexGeometry(gVerts);
}

//add a vertex that is not part of the geometry, but is used by
//ex. LandmarkDimension as a reference
std::string DrawViewPart::addReferenceVertex(Base::Vector3d v)
{
    //    Base::Console().Message("DVP::addReferenceVertex(%s) - %s\n",
    //                            DrawUtil::formatVector(v).c_str(), _getNameInDocument());
    std::string refTag;
    //    Base::Vector3d scaledV = v * getScale();
    //    TechDraw::Vertex* ref = new TechDraw::Vertex(scaledV);
    Base::Vector3d scaledV = v;
    VertexPtr ref(std::make_shared<TechDraw::Vertex>(scaledV));
    ref->isReference(true);
    refTag = ref->getTagAsString();
    m_referenceVerts.push_back(ref);
    return refTag;
}

void DrawViewPart::removeReferenceVertex(std::string tag)
{
    std::vector<VertexPtr> newRefVerts;
    for (auto& v : m_referenceVerts) {
        if (v->getTagAsString() != tag) {
            newRefVerts.push_back(v);
        }
    }
    m_referenceVerts = newRefVerts;
    resetReferenceVerts();
}

void DrawViewPart::removeAllReferencesFromGeom()
{
    auto lock = concurrentData.continueWriting();
    if(!lock) {
        return;
    }

    if (!m_referenceVerts.empty()) {
        std::vector<VertexPtr> gVerts = getVertexGeometry();
        std::vector<VertexPtr> newVerts;
        for (auto& gv: gVerts) {
            if (!gv->isReference()) {
                newVerts.emplace_back(std::move(gv));
            }
        }
        // TODO: newVerts should be moved!
        lock->geometryObject.setVertexGeometry(newVerts);
    }
}

void DrawViewPart::resetReferenceVerts()
{
    //    Base::Console().Message("DVP::resetReferenceVerts() %s\n", _getNameInDocument());
    removeAllReferencesFromGeom();
    addReferencesToGeom();
}

// debugging ----------------------------------------------------------------------------

void DrawViewPart::dumpVerts(std::string text)
{
    auto lock = concurrentData.lockForReading();

    std::vector<TechDraw::VertexPtr> gVerts = getVertexGeometry();
    Base::Console().Message("%s - dumping %d vertGeoms\n", text.c_str(), gVerts.size());
    for (auto& gv : gVerts) {
        gv->dump();
    }
}

void DrawViewPart::dumpCosVerts(std::string text)
{
    std::vector<TechDraw::CosmeticVertex*> cVerts = CosmeticVertexes.getValues();
    Base::Console().Message("%s - dumping %d CosmeticVertexes\n", text.c_str(), cVerts.size());
    for (auto& cv : cVerts) {
        cv->dump("a CV");
    }
}

void DrawViewPart::dumpCosEdges(std::string text)
{
    std::vector<TechDraw::CosmeticEdge*> cEdges = CosmeticEdges.getValues();
    Base::Console().Message("%s - dumping %d CosmeticEdge\n", text.c_str(), cEdges.size());
    for (auto& ce : cEdges) {
        ce->dump("a CE");
    }
}

PyObject* DrawViewPart::getPyObject()
{
    if (PythonObject.is(Py::_None())) {
        // ref counter is set to 1
        PythonObject = Py::Object(new DrawViewPartPy(this), true);
    }
    return Py::new_reference_to(PythonObject);
}

// Python Drawing feature ---------------------------------------------------------

namespace App
{
/// @cond DOXERR
PROPERTY_SOURCE_TEMPLATE(TechDraw::DrawViewPartPython, TechDraw::DrawViewPart)
template<> const char* TechDraw::DrawViewPartPython::getViewProviderName() const
{
    return "TechDrawGui::ViewProviderViewPart";
}
/// @endcond

// explicit template instantiation
template class TechDrawExport FeaturePythonT<TechDraw::DrawViewPart>;
}// namespace App
