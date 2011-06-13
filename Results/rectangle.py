#!/usr/bin/python
import pylab
import os
import numpy
import vtk
import math

def Get_Zero(prof):
    zero=0
    #pylab.figure()
    #pylab.plot(prof)
    for counter in range(0, len(prof)):
        if prof[counter]<0 and prof[counter+1]>=0:
            zero=-(prof[counter]*(counter+1)-prof[counter+1]*counter)/(prof[counter+1]-prof[counter])
    return zero+0.5


def extract_profiles(name):
    
    gridreader = vtk.vtkXMLStructuredGridReader()
    gridreader.SetFileName(name)
    gridreader.Update()
    
    grid  = gridreader.GetOutput()
    data  = grid.GetPointData()
    points=grid.GetPoints()
    dims  =grid.GetDimensions()
    velocity=data.GetArray("Velocity")
    phase=data.GetArray("Phase")
    vz=numpy.zeros([dims[1],dims[2]])
    vy=numpy.zeros([dims[1],dims[2]])
    vx=numpy.zeros([dims[1],dims[2]])
    phase_numpy=numpy.zeros([dims[1],dims[2]])
    
    print dims

    for coory in range(0,dims[1]):
        for coorz in range(0,dims[2]):
            counter=coorz*dims[0]*dims[1]+coory*dims[0]
            velx,vely,velz=velocity.GetTuple3(counter)
            vz[coory,coorz]=velz
            vy[coory,coorz]=vely
            vx[coory,coorz]=velx
            phase_numpy[coory,coorz]=phase.GetTuple1(counter)

    #parameters of the binary liquid model
    k=0.04
    a=0.04
    
    center=phase_numpy[0,:]
    z1 = numpy.min(numpy.where(center < 0.0))
    z2 = numpy.max(numpy.where(center < 0.0))
    if z1==0:
        z2=numpy.min(numpy.where(center>0.0))+dims[2]
        z1=numpy.max(numpy.where(center>0.0))
    print z1,z2
    
    mid =((z1+z2)/2)%dims[2]
    print mid

    slug_mid=((z1+z2+dims[2])/2)%dims[2]
    print "Slug_mid=",slug_mid
    phase_mid=numpy.zeros([dims[0],dims[1]])    
    vel_slug=numpy.zeros([dims[0],dims[1]])    
    #for counter in range(0,points.GetNumberOfPoints()):
    #    coorx,coory,coorz=points.GetPoint(counter)
    #    if coorz==mid:
    #       phase_mid[int(coorx),int(coory)]=phase.GetTuple1(counter)
    for coorx in range(0,dims[0]):
        for coory in range(0,dims[1]):
            counter=mid*dims[0]*dims[1]+coory*dims[0]+coorx
            phase_mid[coorx,coory]= phase.GetTuple1(counter)
            counter=slug_mid*dims[0]*dims[1]+coory*dims[0]+coorx
            vel_slug[coorx,coory]= velocity.GetTuple3(counter)[2]
	    
       
    #pylab.figure()
    #pylab.imshow(phase_mid[1:,1:],cmap="gray",extent=[0.0,1.0,0.0,1.0],origin="lower")
    
    #Calculation of the capillary number
    capillary=vz[0,z2%dims[2]]*(2.0/3.0)/math.sqrt(8.0*k*a/9.0)    
    print "Capillary=",capillary

   # number=str(capillary*100)[:2]
   # if number[1]==".":
   #     number=number[:1]
    
    #pylab.savefig("phase_crossection_ca"+number+".eps")    
    #pylab.imshow(array['phi'],cmap="gray",extent=[0.0,15.0,0.0,1.0])
    #pylab.yticks([0.0,0.5,1.0])

    reynolds=vz[0,z2%dims[2]]*2.0*(dims[0]-2)/(2.0/3.0)
    print "Reynolds=",vz[0,z2%dims[2]]*2.0*(dims[1]-2)/(2.0/3.0)
    prof_axis_x=phase_mid[0,1:]
    prof_axis_y=phase_mid[1:,0]
  
    print Get_Zero(prof_axis_x)
    print Get_Zero(prof_axis_y)
    axis_zero_x=Get_Zero(prof_axis_x)/(dims[1]-2.0)
    axis_zero_y=Get_Zero(prof_axis_y)/(dims[1]-2.0)
    #diag_zero=math.sqrt(2.0)*Get_Zero(prof_diag)/(dims[0]-2.0)
    
    
    #print axis_zero,diag_zero
    #print "Capillary=",capillary

    #pylab.figure()
    #pylab.plot(phase_mid[1,1:])    
    #pylab.plot(numpy.diag(phase_mid[1:,1:]))
    
    #area=numpy.ones(phase_mid.shape)    
    #liq=numpy.where(phase_mid>0.0)
    #bubble=numpy.where(phase_mid<0.0)
    #vol_bubble=numpy.sum(velz_mid[bubble])
    #vol_liq=numpy.sum(velz_mid[liq])
    #vol_pos=numpy.sum(velz_bulk)
    #area_bubble=numpy.sum(area[bubble])
    #area_liq=numpy.sum(area[liq])
    
    #superficial=float(numpy.sum(vel_slug[1:-1,1:-1]))/float((dims[0]-2.5)*(dims[1]-2.5))    
    #print "Interface=",vz[0,z2%dims[2]]
    #print "Superficial=",superficial
    #return axis_zero,diag_zero,capillary,reynolds,vz[0,z2%dims[2]],superficial
    return capillary,axis_zero_x,axis_zero_y
    
def get_radii_rect12():
    dirs=['1','2','4','6','7','8','9','10']
    capillaries=[]
    axis_zeros_x=[]
    axis_zeros_y=[]
    for dir in dirs:
        name="Rectangle12/"+dir+"/phase240000.vts"
        print name
        cap,axis_x,axis_y=extract_profiles(name)
        capillaries.append(cap)
        axis_zeros_x.append(axis_x)
        axis_zeros_y.append(axis_y)
    pylab.figure(1)
    pylab.plot(capillaries,axis_zeros_x,'k-')
    pylab.plot(capillaries,axis_zeros_y,'k--')
    numpy.savetxt("rectangle12.txt",zip(capillaries,axis_zeros_x,axis_zeros_y))

def get_radii_rect14():
    dirs=['3','4','5','7','8','9']
    capillaries=[]
    axis_zeros_x=[]
    axis_zeros_y=[]
    for dir in dirs:
        name="Rectangle14/"+dir+"/phase240000.vts"
        print name
        cap,axis_x,axis_y=extract_profiles(name)
        capillaries.append(cap)
        axis_zeros_x.append(axis_x)
        axis_zeros_y.append(axis_y)
    pylab.figure(1)
    pylab.plot(capillaries,axis_zeros_x,'k^')
    pylab.plot(capillaries,axis_zeros_y,'kv')
    numpy.savetxt("rectangle14.txt",zip(capillaries,axis_zeros_x,axis_zeros_y))


def get_radii_rect16():
    dirs=['1','2','3','4','5','6']
    capillaries=[]
    axis_zeros_x=[]
    axis_zeros_y=[]
    for dir in dirs:
        name="Rectangle16/"+dir+"/phase240000.vts"
        print name
        cap,axis_x,axis_y=extract_profiles(name)
        capillaries.append(cap)
        axis_zeros_x.append(axis_x)
        axis_zeros_y.append(axis_y)
    pylab.figure(1)
    pylab.plot(capillaries,axis_zeros_x,'k^')
    pylab.plot(capillaries,axis_zeros_y,'kv')
    numpy.savetxt("rectangle16.txt",zip(capillaries,axis_zeros_x,axis_zeros_y))

def get_radii_rect18():
    dirs=['1','3','4','5','6']
    capillaries=[]
    axis_zeros_x=[]
    axis_zeros_y=[]
    for dir in dirs:
        name="Rectangle18/"+dir+"/phase240000.vts"
        print name
        cap,axis_x,axis_y=extract_profiles(name)
        capillaries.append(cap)
        axis_zeros_x.append(axis_x)
        axis_zeros_y.append(axis_y)
    pylab.figure(1)
    pylab.plot(capillaries,axis_zeros_x,'k^')
    pylab.plot(capillaries,axis_zeros_y,'kv')
    numpy.savetxt("rectangle18.txt",zip(capillaries,axis_zeros_x,axis_zeros_y))


if __name__=="__main__":
    #get_radii_rect12()
    #get_radii_rect14()
    get_radii_rect16()
    get_radii_rect18()
    pylab.show()
