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

#ifndef GEOMETRIC_SHAPES_SHAPE_OPERATIONS_
#define GEOMETRIC_SHAPES_SHAPE_OPERATIONS_

#include "geometric_shapes/shapes.h"
#include "geometric_shapes/shape_messages.h"
#include <eigen_stl_containers/eigen_stl_containers.h>
#include <visualization_msgs/Marker.h>
#include <vector>

// forward declaration of aiScene (caller needs to include assimp)
class aiScene;

namespace shapes
{

/** \brief Load a mesh from a set of vertices. Triangles are
    constructed using index values from the triangles
    vector. Triangle k has vertices at index values triangles[3k],
    triangles[3k+1], triangles[3k+2]  */
shapes::Mesh* createMeshFromVertices(const EigenSTL::vector_Vector3d &vertices, const std::vector<unsigned int> &triangles);

/** \brief Load a mesh from a set of vertices. Every 3 vertices
    are considered a triangle. Repeating vertices are identified
    and the set of triangle indices is constructed. The normal at
    each triangle is also computed */
shapes::Mesh* createMeshFromVertices(const EigenSTL::vector_Vector3d &source);

/** \brief Load a mesh from a resource that contains a mesh that can be loaded by assimp */
shapes::Mesh* createMeshFromResource(const std::string& resource);

/** \brief Load a mesh from a resource that contains a mesh that can be loaded by assimp */
shapes::Mesh* createMeshFromResource(const std::string& resource, const Eigen::Vector3d &scale);

/** \brief Load a mesh from a binary stream that contains a mesh that can be loaded by assimp */
shapes::Mesh* createMeshFromBinary(const char* buffer, std::size_t size,
                                   const std::string &assimp_hint = std::string());

/** \brief Load a mesh from a resource that contains a mesh that can be loaded by assimp */
shapes::Mesh* createMeshFromBinary(const char *buffer, std::size_t size, const Eigen::Vector3d &scale,
                                   const std::string &assimp_hint = std::string());

/** \brief Load a mesh from an assimp datastructure */
shapes::Mesh* createMeshFromAsset(const aiScene* scene, const Eigen::Vector3d &scale,
                                  const std::string &assimp_hint = std::string());

/** \brief Load a mesh from an assimp datastructure */
shapes::Mesh* createMeshFromAsset(const aiScene* scene,
                                  const std::string &assimp_hint = std::string());

/** \brief Construct the shape that corresponds to the message. Return NULL on failure. */
Shape* constructShapeFromMsg(const shape_msgs::SolidPrimitive &shape_msg);

/** \brief Construct the shape that corresponds to the message. Return NULL on failure. */
Shape* constructShapeFromMsg(const shape_msgs::Plane &shape_msg);

/** \brief Construct the shape that corresponds to the message. Return NULL on failure. */
Shape* constructShapeFromMsg(const shape_msgs::Mesh &shape_msg);

/** \brief Construct the shape that corresponds to the message. Return NULL on failure. */
Shape* constructShapeFromMsg(const ShapeMsg &shape_msg);

/** \brief Construct the message that corresponds to the shape. Return false on failure. */
bool constructMsgFromShape(const Shape* shape, ShapeMsg &shape_msg);

/** \brief Construct the marker that corresponds to the shape. Return false on failure. */
bool constructMarkerFromShape(const Shape* shape, visualization_msgs::Marker &mk, bool use_mesh_triangle_list = false);

/** \brief Compute the extents of a shape */
Eigen::Vector3d computeShapeExtents(const ShapeMsg &shape_msg);

/** \brief Compute the extents of a shape */
Eigen::Vector3d computeShapeExtents(const Shape *shape);

/** \brief Get the string name of the shape */
const std::string& shapeStringName(const Shape *shape);

/** \brief Save all the information about this shape as plain text */
void saveAsText(const Shape *shape, std::ostream &out);

/** \brief Construct a shape from plain text description */
Shape* constructShapeFromText(std::istream &in);

}

#endif
