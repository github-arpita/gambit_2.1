*****************************************************************************
*** auxiliary routine called by dsIBwhdxdy
*** author: Torsten Bringmann, 2007-04-18
*****************************************************************************

      real*8 function dmIBwhdxdy_5(x,y,m0,mw,mhc,mc1)
      real*8 x,y,m0,mw,mhc,mc1

      dmIBwhdxdy_5 = 
     -   (-8*m0**3*mc1*(-2*mc1**2 + mhc**2 + mw**2 + m0**2*(-2+4*x))*
     -    ((-(mhc**2*mw) + mw**3)**2*(mw**2 - mhc**2*x) + 
     -      256*m0**8*(-1 + x)*(x - y)*y + 
     -      4*m0**2*(mhc**6*x*y - mhc**4*mw**2*(x*(-3 + y) + 2*y) - 
     -         mhc**2*mw**4*(2 + (-4 + x)*y) + 
     -         mw**6*(-2 + x - 2*x**2 - 2*y + x*y)) - 
     -      16*m0**4*(mhc**4*y*(-x**2 - y + x*(3 + y)) + 
     -         mhc**2*mw**2*(2*x**2*y + 2*(-2 + y)*y + 
     -            x*(3 + 2*y - 2*y**2)) + 
     -         mw**4*(-1 + 4*x**3 - 4*y - 5*x**2*y - y**2 + 
     -            x*(2 + 3*y + y**2))) - 
     -      64*m0**6*(mhc**2*y*(2*x**2 + 2*y - x*(3 + 2*y)) + 
     -         mw**2*(2*x**4 - 4*x**3*y + 2*y*(1 + y) + 
     -            2*x**2*y*(1 + y) - x*(1 + 3*y + 2*y**2)))))/
     -  (mw**2*(-2*mc1**2 + mhc**2 + 2*mw**2 + m0**2*(-2 + 4*x-4*y))*
     -    (mw**2 + 4*m0**2*(x - y))**2*(mw**2 - 4*m0**2*y)**2*
     -    (-2*mc1**2 + mhc**2 + m0**2*(-2 + 4*y)))
      return
      end   ! dmIBwhdxdy_5
