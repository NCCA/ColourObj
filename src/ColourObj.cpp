#include "ColourObj.h"
#include "boost/spirit.hpp"
#include <ngl/NGLStream.h>
// make a namespace for our parser to save writing boost::spirit:: all the time
namespace spt=boost::spirit;

// syntactic sugar for specifying our grammar
typedef spt::rule<spt::phrase_scanner_t> srule;
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>


ColourObj::ColourObj(const std::string& _fname  , bool _calcBB)  noexcept : ngl::Obj()
{
  m_vbo=false;
  m_ext=0;
  // set default values
  m_nVerts=m_nNorm=m_nTex=m_nFaces=0;
  //set the default extents to 0
  m_maxX=0.0f; m_maxY=0.0f; m_maxZ=0.0f;
  m_minX=0.0f; m_minY=0.0f; m_minZ=0.0f;
  m_nNorm=m_nTex=0;

  // load the file in
  m_loaded=load(_fname,_calcBB);

  m_texture = false;

}



void ColourObj::parseVertex( const char *_begin )  noexcept
{
  //std::cout<<"ColourObj Parse\n";
  std::vector<ngl::Real> values;
  // here is the parse rule to load the data into a vector (above)
  srule vertex = "v" >> spt::real_p[spt::append(values)] >>
                        spt::real_p[spt::append(values)] >>
                        spt::real_p[spt::append(values)] >>
                        spt::real_p[spt::append(values)] >>
                        spt::real_p[spt::append(values)] >>
                        spt::real_p[spt::append(values)];


  // now parse the data
  spt::parse_info<> result = spt::parse(_begin, vertex, spt::space_p);
  // should check this at some stage
  NGL_UNUSED(result);
  // and add it to our vert list in abstact mesh parent
  m_verts.push_back(ngl::Vec3(values[0],values[1],values[2]));
  m_colours.push_back(ngl::Vec3(values[3],values[4],values[5]));

}


void ColourObj::createColourVAO() noexcept
{

  struct VertData
  {
    GLfloat u; // tex cords
    GLfloat v; // tex cords
    GLfloat nx; // normal from obj mesh
    GLfloat ny;
    GLfloat nz;
    GLfloat x; // position from obj
    GLfloat y;
    GLfloat z;
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
  for(unsigned int i=0;i<m_nFaces;++i)
  {
    // now for each triangle in the face (remember we ensured tri above)
    for(unsigned int j=0;j<3;++j)
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
      if(m_nNorm >0 && m_nTex > 0)
      {
        d.nx=m_norm[m_face[i].m_norm[j]].m_x;
        d.ny=m_norm[m_face[i].m_norm[j]].m_y;
        d.nz=m_norm[m_face[i].m_norm[j]].m_z;

        d.u=m_tex[m_face[i].m_tex[j]].m_x;
        d.v=m_tex[m_face[i].m_tex[j]].m_y;
      }
      // now if neither are present (only verts like Zbrush models)
      else if(m_nNorm ==0 && m_nTex==0)
      {
        d.nx=0;
        d.ny=0;
        d.nz=0;
        d.u=0;
        d.v=0;
      }
      // here we've got norms but not tex
      else if(m_nNorm >0 && m_nTex==0)
      {
        d.nx=m_norm[m_face[i].m_norm[j]].m_x;
        d.ny=m_norm[m_face[i].m_norm[j]].m_y;
        d.nz=m_norm[m_face[i].m_norm[j]].m_z;
        d.u=0;
        d.v=0;
      }
      // here we've got tex but not norm least common
      else if(m_nNorm ==0 && m_nTex>0)
      {
        d.nx=0;
        d.ny=0;
        d.nz=0;
        d.u=m_tex[m_face[i].m_tex[j]].m_x;
        d.v=m_tex[m_face[i].m_tex[j]].m_y;
      }
    vboMesh.push_back(d);
    }
  }

  // first we grab an instance of our VOA
  m_vaoMesh.reset( ngl::VAOFactory::createVAO("simpleVAO",m_dataPackType));
  // next we bind it so it's active for setting data
  m_vaoMesh->bind();
  m_meshSize=vboMesh.size();

  // now we have our data add it to the VAO, we need to tell the VAO the following
  // how much (in bytes) data we are copying
  // a pointer to the first element of data (in this case the address of the first element of the
  // std::vector
  m_vaoMesh->setData(ngl::SimpleVAO::VertexData(m_meshSize*sizeof(VertData),vboMesh[0].u));
  // in this case we have packed our data in interleaved format as follows
  // u,v,nx,ny,nz,x,y,z
  // If you look at the shader we have the following attributes being used
  // attribute vec3 inVert; attribute 0
  // attribute vec2 inUV; attribute 1
  // attribute vec3 inNormal; attribure 2
  // so we need to set the vertexAttributePointer so the correct size and type as follows
  // vertex is attribute 0 with x,y,z(3) parts of type GL_FLOAT, our complete packed data is
  // sizeof(vertData) and the offset into the data structure for the first x component is 5 (u,v,nx,ny,nz)..x
  m_vaoMesh->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(VertData),5);
  // uv same as above but starts at 0 and is attrib 1 and only u,v so 2
  m_vaoMesh->setVertexAttributePointer(1,2,GL_FLOAT,sizeof(VertData),0);
  // normal same as vertex only starts at position 2 (u,v)-> nx
  m_vaoMesh->setVertexAttributePointer(2,3,GL_FLOAT,sizeof(VertData),2);
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

