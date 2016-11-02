#ifndef COLOUROBJ_H_
#define COLOUROBJ_H_

#include <ngl/Obj.h>
#include <ngl/Vec3.h>

class ColourObj : public  ngl::Obj
{
  public :
    ColourObj() noexcept : Obj() {}
    explicit ColourObj( const std::string& _fname ,bool _calcBB=true) noexcept;

    void createColourVAO() noexcept;
    //void draw();

  protected :
    void parseVertex( const char *_begin )  noexcept override;
    std::vector <ngl::Vec3> m_colours;
};


#endif
