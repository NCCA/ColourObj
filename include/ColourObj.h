#ifndef COLOUROBJ_H_
#define COLOUROBJ_H_

#include <ngl/Obj.h>
#include <ngl/Vec3.h>

class ColourObj : public  ngl::Obj
{
  public :
    ColourObj() noexcept : Obj() {}
    explicit ColourObj( const std::string& _fname ,ngl::Obj::CalcBB _calcBB=ngl::Obj::CalcBB::True ) noexcept;

    void createColourVAO() noexcept;
    //void draw();

  protected :
    bool parseVertex(std::vector<std::string> &_tokens) noexcept override ;

    std::vector <ngl::Vec3> m_colours;
};


#endif
