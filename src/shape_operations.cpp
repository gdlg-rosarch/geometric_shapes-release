/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2008, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Willow Garage nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Ioan Sucan */

#include "geometric_shapes/shape_operations.h"

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <set>
#include <float.h>

#include <console_bridge/console.h>
#include <resource_retriever/retriever.h>

#if defined(IS_ASSIMP3)
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#else
#include <assimp/aiScene.h>
#include <assimp/assimp.hpp>
#include <assimp/aiPostProcess.h>
#endif

#include <Eigen/Geometry>

#include <shape_tools/shape_to_marker.h>
#include <shape_tools/shape_extents.h>
#include <shape_tools/solid_primitive_dims.h>

namespace shapes
{

namespace detail
{

namespace
{

/// Local representation of a vertex that knows its position in an array (used for sorting)
struct LocalVertexType
{
  LocalVertexType() : x(0.0), y(0.0), z(0.0)
  {
  }
  
  LocalVertexType(const Eigen::Vector3d &v) : x(v.x()), y(v.y()), z(v.z())
  {
  }
  
  double x,y,z;
  unsigned int index;
};

/// Sorting operator by point value
struct ltLocalVertexValue
{
  bool operator()(const LocalVertexType &p1, const LocalVertexType &p2) const
  {
    if (p1.x < p2.x)
      return true;
    if (p1.x > p2.x)
      return false;
    if (p1.y < p2.y)
      return true;
    if (p1.y > p2.y)
      return false;
    if (p1.z < p2.z)
      return true;
    return false;
  }
};

/// Sorting operator by point index
struct ltLocalVertexIndex
{
  bool operator()(const LocalVertexType &p1, const LocalVertexType &p2) const
  {
    return p1.index < p2.index;
  }
};

}
}

Mesh* createMeshFromVertices(const EigenSTL::vector_Vector3d &vertices, const std::vector<unsigned int> &triangles)
{
  unsigned int nt = triangles.size() / 3;
  Mesh *mesh = new Mesh(vertices.size(), nt);
  for (unsigned int i = 0 ; i < vertices.size() ; ++i)
  {
    mesh->vertices[3 * i    ] = vertices[i].x();
    mesh->vertices[3 * i + 1] = vertices[i].y();
    mesh->vertices[3 * i + 2] = vertices[i].z();
  }

  std::copy(triangles.begin(), triangles.end(), mesh->triangles);
  mesh->computeNormals();
  
  return mesh;
}

Mesh* createMeshFromVertices(const EigenSTL::vector_Vector3d &source)
{
  if (source.size() < 3)
    return NULL;
  
  if (source.size() % 3 != 0)
    logError("The number of vertices to construct a mesh from is not divisible by 3. Probably constructed triangles will not make sense.");
  
  std::set<detail::LocalVertexType, detail::ltLocalVertexValue> vertices;
  std::vector<unsigned int> triangles;
  
  unsigned int n = source.size() / 3;
  for (unsigned int i = 0 ; i < n ; ++i)
  {
    // check if we have new vertices
    unsigned int i3 = i * 3;
    detail::LocalVertexType vt1(source[i3]);
    std::set<detail::LocalVertexType, detail::ltLocalVertexValue>::iterator p1 = vertices.find(vt1);
    if (p1 == vertices.end())
    {
      vt1.index = vertices.size();
      vertices.insert(vt1);
    }
    else
      vt1.index = p1->index;
    triangles.push_back(vt1.index);
    
    detail::LocalVertexType vt2(source[++i3]);
    std::set<detail::LocalVertexType, detail::ltLocalVertexValue>::iterator p2 = vertices.find(vt2);
    if (p2 == vertices.end())
    {
      vt2.index = vertices.size();
      vertices.insert(vt2);
    }
    else
      vt2.index = p2->index;
    triangles.push_back(vt2.index);
    
    detail::LocalVertexType vt3(source[++i3]);
    std::set<detail::LocalVertexType, detail::ltLocalVertexValue>::iterator p3 = vertices.find(vt3);
    if (p3 == vertices.end())
    {
      vt3.index = vertices.size();
      vertices.insert(vt3);
    }
    else
      vt3.index = p3->index;
    
    triangles.push_back(vt3.index);
  }

  // sort our vertices
  std::vector<detail::LocalVertexType> vt;
  vt.insert(vt.end(), vertices.begin(), vertices.end());
  std::sort(vt.begin(), vt.end(), detail::ltLocalVertexIndex());

  // copy the data to a mesh structure
  unsigned int nt = triangles.size() / 3;

  Mesh *mesh = new Mesh(vt.size(), nt);
  for (unsigned int i = 0 ; i < vt.size() ; ++i)
  {    
    unsigned int i3 = i * 3;
    mesh->vertices[i3    ] = vt[i].x;
    mesh->vertices[i3 + 1] = vt[i].y;
    mesh->vertices[i3 + 2] = vt[i].z;
  }

  std::copy(triangles.begin(), triangles.end(), mesh->triangles);
  mesh->computeNormals();
  
  return mesh;
}

Mesh* createMeshFromResource(const std::string& resource)
{
  static const Eigen::Vector3d one(1.0, 1.0, 1.0);
  return createMeshFromResource(resource, one);
}

Mesh* createMeshFromBinary(const char* buffer, std::size_t size,
                           const std::string &assimp_hint)
{
  static const Eigen::Vector3d one(1.0, 1.0, 1.0);
  return createMeshFromBinary(buffer, size, one, assimp_hint);
}

Mesh* createMeshFromBinary(const char *buffer, std::size_t size, const Eigen::Vector3d &scale,
                           const std::string &assimp_hint)
{
  if (!buffer || size < 1)
  {
    logWarn("Cannot construct mesh from empty binary buffer");
    return NULL;
  }
  
  // try to get a file extension
  std::string hint;
  std::size_t pos = assimp_hint.find_last_of(".");
  if (pos != std::string::npos)
  {
    hint = assimp_hint.substr(pos + 1);
    std::transform(hint.begin(), hint.end(), hint.begin(), ::tolower);
    if (hint.find("stl") != std::string::npos)
      hint = "stl";
  }
  
  // Create an instance of the Importer class
  Assimp::Importer importer;
  

  // And have it read the given file with some postprocessing
  const aiScene* scene = importer.ReadFileFromMemory(reinterpret_cast<const void*>(buffer), size,
                                                     aiProcess_Triangulate            |
                                                     aiProcess_JoinIdenticalVertices  |
                                                     aiProcess_SortByPType            |
                                                     aiProcess_OptimizeGraph          |
                                                     aiProcess_OptimizeMeshes, assimp_hint.c_str());
  if (scene)
    return createMeshFromAsset(scene, scale, assimp_hint);
  else
    return NULL;
}

Mesh* createMeshFromResource(const std::string& resource, const Eigen::Vector3d &scale)
{
  resource_retriever::Retriever retriever;
  resource_retriever::MemoryResource res;
  try
  {
    res = retriever.get(resource);
  } 
  catch (resource_retriever::Exception& e)
  {
    logError("%s", e.what());
    return NULL;
  }

  if (res.size == 0)
  {
    logWarn("Retrieved empty mesh for resource '%s'", resource.c_str());
    return NULL;
  }
  
  Mesh *m = createMeshFromBinary(reinterpret_cast<const char*>(res.data.get()), res.size, scale, resource);
  if (!m)
    logWarn("Assimp reports no scene in %s", resource.c_str());
  return m;
}

namespace
{
void extractMeshData(const aiScene *scene, const aiNode *node, const aiMatrix4x4 &parent_transform, const Eigen::Vector3d &scale,
                     EigenSTL::vector_Vector3d &vertices, std::vector<unsigned int> &triangles)
{
  aiMatrix4x4 transform = parent_transform;
  transform *= node->mTransformation;
  for (unsigned int j = 0 ; j < node->mNumMeshes; ++j)
  {
    const aiMesh* a = scene->mMeshes[node->mMeshes[j]];   
    unsigned int offset = vertices.size();    
    for (unsigned int i = 0 ; i < a->mNumVertices ; ++i)
    {
      aiVector3D v = transform * a->mVertices[i];
      vertices.push_back(Eigen::Vector3d(v.x * scale.x(), v.y * scale.y(), v.z * scale.z()));
    }
    for (unsigned int i = 0 ; i < a->mNumFaces ; ++i)
      if (a->mFaces[i].mNumIndices == 3)
      {
        triangles.push_back(offset + a->mFaces[i].mIndices[0]);
        triangles.push_back(offset + a->mFaces[i].mIndices[1]);
        triangles.push_back(offset + a->mFaces[i].mIndices[2]);
      }
  }
  
  for (unsigned int n = 0; n < node->mNumChildren; ++n)
    extractMeshData(scene, node->mChildren[n], transform, scale, vertices, triangles);
}
}

Mesh* createMeshFromAsset(const aiScene* scene, const std::string &resource_name)
{
  static const Eigen::Vector3d one(1.0, 1.0, 1.0);
  return createMeshFromAsset(scene, one, resource_name);
}

Mesh* createMeshFromAsset(const aiScene* scene, const Eigen::Vector3d &scale, const std::string &resource_name)
{
  if (!scene->HasMeshes())
  {
    logWarn("Assimp reports scene in %s has no meshes", resource_name.c_str());
    return NULL;
  }
  EigenSTL::vector_Vector3d vertices;
  std::vector<unsigned int> triangles;
  extractMeshData(scene, scene->mRootNode, aiMatrix4x4(), scale, vertices, triangles);
  if (vertices.empty())
  {
    logWarn("There are no vertices in the scene %s", resource_name.c_str());
    return NULL;
  }
  if (triangles.empty())
  {
    logWarn("There are no triangles in the scene %s", resource_name.c_str());
    return NULL;
  }
  
  return createMeshFromVertices(vertices, triangles);
}

Shape* constructShapeFromMsg(const shape_msgs::Plane &shape_msg)
{ 
  return new Plane(shape_msg.coef[0], shape_msg.coef[1], shape_msg.coef[2], shape_msg.coef[3]);
}

Shape* constructShapeFromMsg(const shape_msgs::Mesh &shape_msg)
{      
  if (shape_msg.triangles.empty() || shape_msg.vertices.empty())
  {
    logWarn("Mesh definition is empty");
    return NULL;
  }
  else
  {
    EigenSTL::vector_Vector3d vertices(shape_msg.vertices.size());
    std::vector<unsigned int> triangles(shape_msg.triangles.size() * 3);
    for (unsigned int i = 0 ; i < shape_msg.vertices.size() ; ++i)
      vertices[i] = Eigen::Vector3d(shape_msg.vertices[i].x, shape_msg.vertices[i].y, shape_msg.vertices[i].z);
    for (unsigned int i = 0 ; i < shape_msg.triangles.size() ; ++i)
    {
      unsigned int i3 = i * 3;
      triangles[i3++] = shape_msg.triangles[i].vertex_indices[0];
      triangles[i3++] = shape_msg.triangles[i].vertex_indices[1];
      triangles[i3] = shape_msg.triangles[i].vertex_indices[2];
    }
    return createMeshFromVertices(vertices, triangles);
  }
}

Shape* constructShapeFromMsg(const shape_msgs::SolidPrimitive &shape_msg)
{
  Shape *shape = NULL;
  if (shape_msg.type == shape_msgs::SolidPrimitive::SPHERE)
  {
    if (shape_msg.dimensions.size() > shape_msgs::SolidPrimitive::SPHERE_RADIUS)
      shape = new Sphere(shape_msg.dimensions[shape_msgs::SolidPrimitive::SPHERE_RADIUS]);
  }
  else
    if (shape_msg.type == shape_msgs::SolidPrimitive::BOX)
    {
      if (shape_msg.dimensions.size() > shape_msgs::SolidPrimitive::BOX_X &&
          shape_msg.dimensions.size() > shape_msgs::SolidPrimitive::BOX_Y &&
          shape_msg.dimensions.size() > shape_msgs::SolidPrimitive::BOX_Z)
        shape = new Box(shape_msg.dimensions[shape_msgs::SolidPrimitive::BOX_X],
                        shape_msg.dimensions[shape_msgs::SolidPrimitive::BOX_Y],
                        shape_msg.dimensions[shape_msgs::SolidPrimitive::BOX_Z]);
    }
    else
      if (shape_msg.type == shape_msgs::SolidPrimitive::CYLINDER)
      {    
        if (shape_msg.dimensions.size() > shape_msgs::SolidPrimitive::CYLINDER_RADIUS && 
            shape_msg.dimensions.size() > shape_msgs::SolidPrimitive::CYLINDER_HEIGHT)
          shape = new Cylinder(shape_msg.dimensions[shape_msgs::SolidPrimitive::CYLINDER_RADIUS],
                               shape_msg.dimensions[shape_msgs::SolidPrimitive::CYLINDER_HEIGHT]);
      }
      else
        if (shape_msg.type == shape_msgs::SolidPrimitive::CONE)
        {
          if (shape_msg.dimensions.size() > shape_msgs::SolidPrimitive::CONE_RADIUS && 
              shape_msg.dimensions.size() > shape_msgs::SolidPrimitive::CONE_HEIGHT)
            shape = new Cone(shape_msg.dimensions[shape_msgs::SolidPrimitive::CONE_RADIUS],
                             shape_msg.dimensions[shape_msgs::SolidPrimitive::CONE_HEIGHT]);
        }
  if (shape == NULL)
    logError("Unable to construct shape corresponding to shape_msgect of type %d", (int)shape_msg.type);
  
  return shape;
}

namespace
{

class ShapeVisitorAlloc : public boost::static_visitor<Shape*>
{
public:
  
  Shape* operator()(const shape_msgs::Plane &shape_msg) const
  {
    return constructShapeFromMsg(shape_msg);
  }
  
  Shape* operator()(const shape_msgs::Mesh &shape_msg) const
  {
    return constructShapeFromMsg(shape_msg);
  }
  
  Shape* operator()(const shape_msgs::SolidPrimitive &shape_msg) const
  {
    return constructShapeFromMsg(shape_msg);
  }
};

}

Shape* constructShapeFromMsg(const ShapeMsg &shape_msg)
{
  return boost::apply_visitor(ShapeVisitorAlloc(), shape_msg);
}

namespace
{

class ShapeVisitorMarker : public boost::static_visitor<void>
{
public:
  
  ShapeVisitorMarker(visualization_msgs::Marker *marker, bool use_mesh_triangle_list) :
    boost::static_visitor<void>(),
    use_mesh_triangle_list_(use_mesh_triangle_list),
    marker_(marker)
  {
  }
  
  void operator()(const shape_msgs::Plane &shape_msg) const
  {
    throw std::runtime_error("No visual markers can be constructed for planes");
  }
  
  void operator()(const shape_msgs::Mesh &shape_msg) const
  {
    shape_tools::constructMarkerFromShape(shape_msg, *marker_, use_mesh_triangle_list_);
  }
  
  void operator()(const shape_msgs::SolidPrimitive &shape_msg) const
  {
    shape_tools::constructMarkerFromShape(shape_msg, *marker_);
  }
  
private:
  
  bool use_mesh_triangle_list_;
  visualization_msgs::Marker *marker_;
};

}

bool constructMarkerFromShape(const Shape* shape, visualization_msgs::Marker &marker, bool use_mesh_triangle_list)
{
  ShapeMsg shape_msg;
  if (constructMsgFromShape(shape, shape_msg))
  {
    bool ok = false;
    try
    { 
      boost::apply_visitor(ShapeVisitorMarker(&marker, use_mesh_triangle_list), shape_msg);
      ok = true;
    }
    catch (std::runtime_error &ex)
    {
      logError("%s", ex.what());
    }
    if (ok)
      return true;
  }
  return false;
}

namespace
{

class ShapeVisitorComputeExtents : public boost::static_visitor<Eigen::Vector3d>
{
public:
    
  Eigen::Vector3d operator()(const shape_msgs::Plane &shape_msg) const
  {
    Eigen::Vector3d e(0.0, 0.0, 0.0);
    return e;
  }
  
  Eigen::Vector3d operator()(const shape_msgs::Mesh &shape_msg) const
  {   
    double x_extent, y_extent, z_extent;
    shape_tools::getShapeExtents(shape_msg, x_extent, y_extent, z_extent);
    Eigen::Vector3d e(x_extent, y_extent, z_extent);
    return e;
  }
  
  Eigen::Vector3d operator()(const shape_msgs::SolidPrimitive &shape_msg) const
  {
    double x_extent, y_extent, z_extent;
    shape_tools::getShapeExtents(shape_msg, x_extent, y_extent, z_extent);
    Eigen::Vector3d e(x_extent, y_extent, z_extent);
    return e;
  }
};

}
  
Eigen::Vector3d computeShapeExtents(const ShapeMsg &shape_msg)
{
  return boost::apply_visitor(ShapeVisitorComputeExtents(), shape_msg);
}

Eigen::Vector3d computeShapeExtents(const Shape *shape)
{
  ShapeMsg shape_msg;
  if (constructMsgFromShape(shape, shape_msg))
    return computeShapeExtents(shape_msg);
  else
    return Eigen::Vector3d(0.0, 0.0, 0.0);
}

bool constructMsgFromShape(const Shape* shape, ShapeMsg &shape_msg)
{
  if (shape->type == SPHERE)
  {
    shape_msgs::SolidPrimitive s;
    s.type = shape_msgs::SolidPrimitive::SPHERE;
    s.dimensions.resize(shape_tools::SolidPrimitiveDimCount<shape_msgs::SolidPrimitive::SPHERE>::value);
    s.dimensions[shape_msgs::SolidPrimitive::SPHERE_RADIUS] = static_cast<const Sphere*>(shape)->radius;
    shape_msg = s;
  }
  else
    if (shape->type == BOX)
    { 
      shape_msgs::SolidPrimitive s;
      s.type = shape_msgs::SolidPrimitive::BOX;
      const double* sz = static_cast<const Box*>(shape)->size;
      s.dimensions.resize(shape_tools::SolidPrimitiveDimCount<shape_msgs::SolidPrimitive::BOX>::value);
      s.dimensions[shape_msgs::SolidPrimitive::BOX_X] = sz[0];
      s.dimensions[shape_msgs::SolidPrimitive::BOX_Y] = sz[1];
      s.dimensions[shape_msgs::SolidPrimitive::BOX_Z] = sz[2];
      shape_msg = s;
    }
    else
      if (shape->type == CYLINDER)
      { 
        shape_msgs::SolidPrimitive s;
        s.type = shape_msgs::SolidPrimitive::CYLINDER;  
        s.dimensions.resize(shape_tools::SolidPrimitiveDimCount<shape_msgs::SolidPrimitive::CYLINDER>::value);
        s.dimensions[shape_msgs::SolidPrimitive::CYLINDER_RADIUS] = static_cast<const Cylinder*>(shape)->radius;
        s.dimensions[shape_msgs::SolidPrimitive::CYLINDER_HEIGHT] = static_cast<const Cylinder*>(shape)->length;
        shape_msg = s;
      }
      else
        if (shape->type == CONE)
        {   
          shape_msgs::SolidPrimitive s;
          s.type = shape_msgs::SolidPrimitive::CONE;
          s.dimensions.resize(shape_tools::SolidPrimitiveDimCount<shape_msgs::SolidPrimitive::CONE>::value);
          s.dimensions[shape_msgs::SolidPrimitive::CONE_RADIUS] = static_cast<const Cone*>(shape)->radius;
          s.dimensions[shape_msgs::SolidPrimitive::CONE_HEIGHT] = static_cast<const Cone*>(shape)->length;
          shape_msg = s;
        }
        else
          if (shape->type == PLANE)
          {    
            shape_msgs::Plane s;
            const Plane *p = static_cast<const Plane*>(shape);
            s.coef[0] = p->a;
            s.coef[1] = p->b;
            s.coef[2] = p->c;
            s.coef[3] = p->d;  
            shape_msg = s;
          }
          else
            if (shape->type == MESH)
            {
              shape_msgs::Mesh s;
              const Mesh *mesh = static_cast<const Mesh*>(shape);
              s.vertices.resize(mesh->vertex_count);
              s.triangles.resize(mesh->triangle_count);
              
              for (unsigned int i = 0 ; i < mesh->vertex_count ; ++i)
              {
                unsigned int i3 = i * 3;
                s.vertices[i].x = mesh->vertices[i3];
                s.vertices[i].y = mesh->vertices[i3 + 1];
                s.vertices[i].z = mesh->vertices[i3 + 2];
              }
              
              for (unsigned int i = 0 ; i < s.triangles.size() ; ++i)
              { 
                unsigned int i3 = i * 3;
                s.triangles[i].vertex_indices[0] = mesh->triangles[i3];
                s.triangles[i].vertex_indices[1] = mesh->triangles[i3 + 1];
                s.triangles[i].vertex_indices[2] = mesh->triangles[i3 + 2];
              }
              shape_msg = s;
            }
            else
            {
              logError("Unable to construct shape message for shape of type %d", (int)shape->type);
              return false;
            }
  
  return true;
}

void saveAsText(const Shape *shape, std::ostream &out)
{
  if (shape->type == SPHERE)
  {
    out << Sphere::STRING_NAME << std::endl;
    out << static_cast<const Sphere*>(shape)->radius << std::endl;
  }
  else
    if (shape->type == BOX)
    {     
      out << Box::STRING_NAME << std::endl;
      const double* sz = static_cast<const Box*>(shape)->size;
      out << sz[0] << " " << sz[1] << " " << sz[2] << std::endl;
    }
    else
      if (shape->type == CYLINDER)
      { 
        out << Cylinder::STRING_NAME << std::endl;
        out << static_cast<const Cylinder*>(shape)->radius << " " << static_cast<const Cylinder*>(shape)->length << std::endl;
      }
      else
        if (shape->type == CONE)
        {     
          out << Cone::STRING_NAME << std::endl;
          out << static_cast<const Cone*>(shape)->radius << " " << static_cast<const Cone*>(shape)->length << std::endl;
        }
        else
          if (shape->type == PLANE)
          {    
            out << Plane::STRING_NAME << std::endl;
            const Plane *p = static_cast<const Plane*>(shape);
            out << p->a << " " << p->b << " " << p->c << " " << p->d << std::endl;
          }
          else
            if (shape->type == MESH)
            {     
              out << Mesh::STRING_NAME << std::endl;
              const Mesh *mesh = static_cast<const Mesh*>(shape);
              out << mesh->vertex_count << " " << mesh->triangle_count << std::endl;
              
              for (unsigned int i = 0 ; i < mesh->vertex_count ; ++i)
              {
                unsigned int i3 = i * 3;
                out << mesh->vertices[i3] << " " << mesh->vertices[i3 + 1] << " " << mesh->vertices[i3 + 2] << std::endl;
              }
              
              for (unsigned int i = 0 ; i < mesh->triangle_count ; ++i)
              { 
                unsigned int i3 = i * 3;
                out << mesh->triangles[i3] << " " << mesh->triangles[i3 + 1] << " " << mesh->triangles[i3 + 2] << std::endl;
              }
            }
            else
            {
              logError("Unable to save shape of type %d", (int)shape->type);
            }
}

Shape* constructShapeFromText(std::istream &in)
{
  Shape *result = NULL;
  if (in.good() && !in.eof())
  {    
    std::string type;
    in >> type;
    if (in.good() && !in.eof())
    {    
      if (type == Sphere::STRING_NAME)
      {
        double radius;
        in >> radius;
        result = new Sphere(radius);
      }
      else
        if (type == Box::STRING_NAME)
        {
          double x, y, z;
          in >> x >> y >> z;
          result = new Box(x, y, z);
        }
        else
          if (type == Cylinder::STRING_NAME)
          {
            double r, l;
            in >> r >> l;
            result = new Cylinder(r, l);
          }
          else
            if (type == Cone::STRING_NAME)
            {
              double r, l;
              in >> r >> l;
              result = new Cone(r, l);
            }   
            else
              if (type == Plane::STRING_NAME)
              {
                double a, b, c, d;
                in >> a >> b >> c >> d;
                result = new Plane(a, b, c, d);
              }
              else
                if (type == Mesh::STRING_NAME)
                {
                  unsigned int v, t;
                  in >> v >> t;
                  Mesh *m = new Mesh(v, t);
                  result = m;
                  for (unsigned int i = 0 ; i < m->vertex_count ; ++i)
                  {
                    unsigned int i3 = i * 3;
                    in >> m->vertices[i3] >> m->vertices[i3 + 1] >> m->vertices[i3 + 2];
                  }
                  for (unsigned int i = 0 ; i < m->triangle_count ; ++i)
                  { 
                    unsigned int i3 = i * 3;
                    in >> m->triangles[i3] >> m->triangles[i3 + 1] >> m->triangles[i3 + 2];
                  }
                  m->computeNormals();
                }
                else
                  logError("Unknown shape type: '%s'", type.c_str());
    }
  }
  return result;
}

const std::string& shapeStringName(const Shape *shape)
{
  static const std::string unknown = "unknown"; 
  if (shape)
    switch (shape->type)
    {
    case SPHERE:
      return Sphere::STRING_NAME;
    case CYLINDER:
      return Cylinder::STRING_NAME;
    case CONE:
      return Cone::STRING_NAME;
    case BOX:
      return Box::STRING_NAME;
    case PLANE:
      return Plane::STRING_NAME;
    case MESH:
      return Mesh::STRING_NAME;
    case OCTREE:
      return OcTree::STRING_NAME;
    default:
      return unknown;
    }
  else
  {
    static const std::string empty;
    return empty;
  }
}

}

