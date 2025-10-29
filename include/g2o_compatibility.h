#pragma once

// g2o 兼容性头文件
// 基于实际 g2o 头文件结构

#include <g2o/types/slam3d/se3_ops.h>
#include <g2o/types/slam3d/edge_se3.h>
#include <g2o/types/slam3d/vertex_se3.h>
#include <g2o/types/slam3d/vertex_pointxyz.h>  // 使用 slam3d 中的 vertex_pointxyz
#include <g2o/types/sba/types_sba.h>
#include <g2o/types/sba/edge_project_xyz.h>
#include <g2o/types/sba/vertex_se3_expmap.h>
#include <g2o/types/sim3/sim3.h>

// 兼容性定义 - 将旧版 g2o 类型映射到新版
namespace g2o {
    // SE3 类型兼容
    using SE3Quat = g2o::SE3Quat;
    
    // 顶点类型兼容
    using VertexSE3Expmap = g2o::VertexSE3ExpMap;
    using VertexSim3Expmap = g2o::VertexSim3Expmap;
    using VertexPointXYZ = g2o::VertexPointXYZ;  // 使用 slam3d 中的 VertexPointXYZ
    
    // 边类型兼容
    using EdgeSE3ProjectXYZ = g2o::EdgeProjectXYZ;
    using EdgeSim3ProjectXYZ = g2o::EdgeSim3ProjectXYZ;
    using EdgeSE3Expmap = g2o::EdgeSE3;
    
    // 使用 g2o 自带的 Sim3 类型
    using Sim3 = g2o::Sim3;
}

// 旧版 g2o 头文件映射
#define G2O_TYPES_SLAM3D_TYPES_SIX_DOF_EXPMAP_H
#define G2O_TYPES_SBA_TYPES_SBA_H
#define G2O_TYPES_SLAM3D_TYPES_SEVEN_DOF_EXPMAP_H