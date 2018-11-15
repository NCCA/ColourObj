#include "ColourObj.h"
#include <ngl/NGLStream.h>

#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/pystring.h>

namespace ps=pystring;


ColourObj::ColourObj(const std::string& _fname  , ngl::Obj::CalcBB _calcBB)  noexcept : ngl::Obj()
{
  if ( load(_fname,_calcBB) == false)
    {
      ngl::NGLMessage::addError(fmt::format("Error loading file {0} ",_fname.data()));
      exit(EXIT_FAILURE);
    }

}



bool ColourObj::parseVertex(std::vector<std::string> &_tokens) noexcept
{
  bool parsedOK=true;
  try
  {
    float x=std::stof(_tokens[1]);
    float y=std::stof(_tokens[2]);
    float z=std::stof(_tokens[3]);
    float r=std::stof(_tokens[4]);
    float g=std::stof(_tokens[5]);
    float b=std::stof(_tokens[6]);


    m_verts.push_back({x,y,z});
    m_colours.push_back({r,g,b});
    ++m_currentVertexOffset;
  }
  catch (std::invalid_argument)
  {
    ngl::NGLMessage::addError("problem converting Obj file vertex");
    parsedOK=false;
  }
  return parsedOK;
}


void ColourObj::createColourVAO() noexcept
{

  struct VertData
  {
    GLfloat x; // position from obj
    GLfloat y;
    GLfloat z;
    GLfloat nx; // normal from obj mesh
    GLfloat ny;
    GLfloat nz;
    GLfloat u; // tex cords
    GLfloat v; // tex cords
    GLfloat r; // colours
    GLfloat g;
    GLfloat b;
  };

  std::cout<<"Colour Obj Create VAO\n";
  // if we have already created a VBO just return.
  if(m_vao == true)
  {
    std::cout<<"VAO exist so returning\n";
    return;
  }
// else allocate space as build our VAO
  m_dataPackType=0;
  if(isTriangular())
  {
    m_dataPackType=GL_TRIANGLES;
    std::cout <<"Doing Tri Data"<<std::endl;
  }
  // data is mixed of > quad so exit error
  if(m_dataPackType == 0)
  {
    std::cerr<<"Can only create VBO from all Triangle or ALL Quad data at present"<<std::endl;
    exit(EXIT_FAILURE);
  }

  // now we are going to process and pack the mesh into an ngl::VertexArrayObject
  std::vector <VertData> vboMesh;
  VertData d;


  // loop for each of the faces
  for(size_t i=0;i<m_face.size();++i)
  {
    // now for each triangle in the face (remember we ensured tri above)
    for(size_t j=0;j<3;++j)
    {

      // pack in the vertex data first
      d.x=m_verts[m_face[i].m_vert[j]].m_x;
      d.y=m_verts[m_face[i].m_vert[j]].m_y;
      d.z=m_verts[m_face[i].m_vert[j]].m_z;
      // set the colours
      d.r=m_colours[m_face[i].m_vert[j]].m_x;
      d.g=m_colours[m_face[i].m_vert[j]].m_y;
      d.b=m_colours[m_face[i].m_vert[j]].m_z;

      // now if we have norms of tex (possibly could not) pack them as well
      if(m_norm.size() >0 && m_uv.size() > 0)
      {
        d.nx=m_norm[m_face[i].m_norm[j]].m_x;
        d.ny=m_norm[m_face[i].m_norm[j]].m_y;
        d.nz=m_norm[m_face[i].m_norm[j]].m_z;

        d.u=m_uv[m_face[i].m_uv[j]].m_x;
        d.v=m_uv[m_face[i].m_uv[j]].m_y;
      }
      // now if neither are present (only verts like Zbrush models)
      else if(m_norm.size() ==0 &&  m_uv.size()==0)
      {
        d.nx=0;
        d.ny=0;
        d.nz=0;
        d.u=0;
        d.v=0;
      }
      // here we've got norms but not tex
      else if(m_norm.size() >0 && m_uv.size()==0)
      {
        d.nx=m_norm[m_face[i].m_norm[j]].m_x;
        d.ny=m_norm[m_face[i].m_norm[j]].m_y;
        d.nz=m_norm[m_face[i].m_norm[j]].m_z;
        d.u=0;
        d.v=0;
      }
      // here we've got tex but not norm least common
      else if(m_norm.size() ==0 && m_uv.size()>0)
      {
        d.nx=0;
        d.ny=0;
        d.nz=0;
        d.u=m_uv[m_face[i].m_uv[j]].m_x;
        d.v=m_uv[m_face[i].m_uv[j]].m_y;
      }
    vboMesh.push_back(d);
    }
  }

  // first we grab an instance of our VOA
  m_vaoMesh=ngl::VAOFactory::createVAO("simpleVAO",m_dataPackType);
  // next we bind it so it's active for setting data
  m_vaoMesh->bind();
  m_meshSize=vboMesh.size();

  // now we have our data add it to the VAO, we need to tell the VAO the following
  // how much (in bytes) data we are copying
  // a pointer to the first element of data (in this case the address of the first element of the
  // std::vector
  m_vaoMesh->setData(ngl::SimpleVAO::VertexData(m_meshSize*sizeof(VertData),vboMesh[0].x));
  // so we need to set the vertexAttributePointer so the correct size and type as follows
  m_vaoMesh->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(VertData),0);
  // uv same as above but starts at 0 and is attrib 1 and only u,v so 2
  m_vaoMesh->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(VertData),3);
  // normal same as vertex only starts at position 2 (u,v)-> nx
  m_vaoMesh->setVertexAttributePointer(2,2,GL_FLOAT,sizeof(VertData),6);
  // set the colour
  m_vaoMesh->setVertexAttributePointer(3,3,GL_FLOAT,sizeof(VertData),8);


  // now we have set the vertex attributes we tell the VAO class how many indices to draw when
  // glDrawArrays is called, in this case we use buffSize (but if we wished less of the sphere to be drawn we could
  // specify less (in steps of 3))
  m_vaoMesh->setNumIndices(m_meshSize);
  // finally we have finished for now so time to unbind the VAO
  m_vaoMesh->unbind();

  // indicate we have a vao now
  m_vao=true;

}

