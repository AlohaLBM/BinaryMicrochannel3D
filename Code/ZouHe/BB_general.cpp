#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <vector>

//Domain size
const int NY=9;
const int NX=11;

//Time steps
const int N=100000;
const int NOUTPUT=1000;

//Fields and populations
double f[NX][NY][9], f2[NX][NY][9];
double rho[NX][NY],ux[NX][NY],uy[NX][NY];

//Pressure boundary conditions
double rho_inlet=1.003;
double rho_outlet=1.0;

//BGK relaxation parameter
double omega=1.0;

//Magic Irina's parameters
double omegaginzburg=8.0*(2.0-omega)/(8.0-omega);
double omegamat[]={1.0,1.0,1.0,omega,omega,omega,1.0,omegaginzburg,omegaginzburg};

//Underlying lattice parameters
double weights[]={4.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/9.0,1.0/36.0,1.0/36.0,1.0/36.0,1.0/36.0};
int cx[]={0,1,0,-1,0,1,-1,-1,1};
int cy[]={0,0,1,0,-1,1,1,-1,-1};
double g[]={1.0,-2.0,-2.0,-2.0,-2.0,4.0,4.0,4.0,4.0};
int compliment[]={0,3,4,1,2,7,8,5,6};

//Matrix M
double M[9][9];


void matrix_init()
{
	for (int iCoor=0;iCoor<9;iCoor++)
	{
		M[0][iCoor]=1.0;
		M[1][iCoor]=cx[iCoor]*sqrt(3.0);
		M[2][iCoor]=cy[iCoor]*sqrt(3.0);
		M[3][iCoor]=(cx[iCoor]*cx[iCoor]-1.0/3.0)*3.0/sqrt(2.0);
		M[4][iCoor]=cx[iCoor]*cy[iCoor]*3.0;
		M[5][iCoor]=(cy[iCoor]*cy[iCoor]-1.0/3.0)*3.0/sqrt(2.0);
		M[6][iCoor]=g[iCoor]/2.0;
		M[7][iCoor]=g[iCoor]*cx[iCoor]*sqrt(1.5)/2.0;
		M[8][iCoor]=g[iCoor]*cy[iCoor]*sqrt(1.5)/2.0;
	}
}

void writedensity(std::string const & fName)
{
	std::string fullName = "./tmp/" + fName+ ".dat";
	std::ofstream fout(fullName.c_str());
	fout.precision(10);

	for (int iY=NY-1; iY>=0; --iY)
	{
		for (int iX=0; iX<NX; ++iX)
			fout<<rho[iX][iY]<<" ";
		fout<<"\n";
	}

}

void writevelocity(std::string const & fName)
{
	std::string fullName = "./tmp/" + fName+ ".dat";
	std::ofstream fout(fullName.c_str());
	fout.precision(10);

	for (int iY=NY-1; iY>=0; --iY)
	{
		for (int iX=0; iX<NX; ++iX)
			fout<<ux[iX][iY]<<" ";
		fout<<"\n";
	}

}

void init()
{
	//Initialization of initial conditions

	//BB and Bulk nodes initialization
	for(int iX=0;iX<NX;iX++)
		for(int iY=0;iY<NY;iY++)
		{
			rho[iX][iY]=1.0;
			ux[iX][iY]=0.0;
			uy[iX][iY]=0.0;

			double fluxx=3.0*ux[iX][iY];
			double fluxy=3.0*uy[iX][iY];

			double qxx=4.5*ux[iX][iY]*ux[iX][iY];
			double qxy=9.0*ux[iX][iY]*uy[iX][iY];
			double qyy=4.5*uy[iX][iY]*uy[iX][iY];

			for(int iPop=0;iPop<9;iPop++)
			{
				f[iX][iY][iPop]=weights[iPop]*(rho[iX][iY]+fluxx*cx[iPop]+fluxy*cy[iPop]+
								qxx*(cx[iPop]*cx[iPop]-1.0/3.0)+qxy*cx[iPop]*cy[iPop]+qyy*(cy[iPop]*cy[iPop]-1.0/3.0));
			}
		}

	//Pressure boundary nodes initialization
	for(int iY=1;iY<NY-1;iY++)
	{
		rho[0][iY]=rho_inlet;
		ux[0][iY]=0.0;
		uy[0][iY]=0.0;

		double fluxx=3.0*ux[0][iY];
		double fluxy=3.0*uy[0][iY];

		double qxx=4.5*ux[0][iY]*ux[0][iY];
		double qxy=9.0*ux[0][iY]*uy[0][iY];
		double qyy=4.5*uy[0][iY]*uy[0][iY];

		for(int iPop=0;iPop<9;iPop++)
		{
			f[0][iY][iPop]=weights[iPop]*(rho[0][iY]+fluxx*cx[iPop]+fluxy*cy[iPop]+
							qxx*(cx[iPop]*cx[iPop]-1.0/3.0)+qxy*cx[iPop]*cy[iPop]+qyy*(cy[iPop]*cy[iPop]-1.0/3.0));
		}


   		rho[NX-1][iY]=rho_outlet;
		ux[NX-1][iY]=0.0;
		uy[NX-1][iY]=0.0;

		fluxx=3.0*ux[NX-1][iY];
		fluxy=3.0*uy[NX-1][iY];

		qxx=4.5*ux[NX-1][iY]*ux[NX-1][iY];
		qxy=9.0*ux[NX-1][iY]*uy[NX-1][iY];
		qyy=4.5*uy[NX-1][iY]*uy[NX-1][iY];

		for(int iPop=0;iPop<9;iPop++)
		{
			f[NX-1][iY][iPop]=weights[iPop]*(rho[NX-1][iY]+fluxx*cx[iPop]+fluxy*cy[iPop]+
							qxx*(cx[iPop]*cx[iPop]-1.0/3.0)+qxy*cx[iPop]*cy[iPop]+qyy*(cy[iPop]*cy[iPop]-1.0/3.0));
		}
	}

}

void general_pressure(int xCoor, int normx, int normy,double rho_pressure)
{
    std::vector<int> unknown;
    std::vector<int> known;
    std::vector<int> positive;
    std::vector<int> negative;
    std::vector<int> neutral;
    std::vector<int> diagonal;

    for(int iCoor=0;iCoor<9;iCoor++)
    {
        //Determination of the projection
        int dot_product=cx[iCoor]*normx+cy[iCoor]*normy;

        if (dot_product>0)
        {
            unknown.push_back(iCoor);
            positive.push_back(iCoor);
            if (! ((cx[iCoor]==normx)&&(cy[iCoor]==normy)) )
            {
                diagonal.push_back(iCoor);
            }
        }
        else
        {
            known.push_back(iCoor);
            if (dot_product<0)
                negative.push_back(iCoor);
            else
                neutral.push_back(iCoor);
        }
    }

//    for(int iCoor=0;iCoor<neutral.size();iCoor++)
//    {
//        std::cout<<"Neutral populations="<<neutral[iCoor]<<"\n";
//    }
//    std::cout<<"\n";

    //Update macroscopic fields
    for (int iY=1;iY<NY-1;iY++)
    {
        rho[xCoor][iY]=rho_pressure;

        double denseminus=0.0;
        double denseneutral=0.0;

        for(int iCount=0;iCount<negative.size();iCount++)
            denseminus+=f[xCoor][iY][negative[iCount]];

        for(int iCount=0;iCount<neutral.size();iCount++)
            denseneutral+=f[xCoor][iY][neutral[iCount]];

        double norma=sqrt(normx*normx+normy*normy);

        double velocity_perp=(-2.0*denseminus-denseneutral+rho_pressure)/rho_pressure;

        ux[xCoor][iY]=velocity_perp*double(normx)/norma;
        uy[xCoor][iY]=velocity_perp*double(normy)/norma;

        double rho_temp=rho[xCoor][iY];
        double ux_temp=ux[xCoor][iY];
        double uy_temp=uy[xCoor][iY];

        //Perform non-equilibrium BB for the unknown populations
        for(int iCount=0;iCount<unknown.size();iCount++)
        {
            int pos=unknown[iCount];


            double eqpos=weights[pos]*(rho_temp + 3.0*rho_temp*(cx[pos]*ux_temp+cy[pos]*uy_temp)
                            +4.5*rho_temp*((cx[pos]*cx[pos]-1.0/3.0)*ux_temp*ux_temp
                                             +(cy[pos]*cy[pos]-1.0/3.0)*uy_temp*uy_temp
                                             +2.0*ux_temp*uy_temp*cx[pos]*cy[pos]));

            int neg=compliment[unknown[iCount]];
            double eqneg=weights[neg]*(rho_temp + 3.0*rho_temp*(cx[neg]*ux_temp+cy[neg]*uy_temp)
                            +4.5*rho_temp*((cx[neg]*cx[neg]-1.0/3.0)*ux_temp*ux_temp
                                             +(cy[neg]*cy[neg]-1.0/3.0)*uy_temp*uy_temp
                                             +2.0*ux_temp*uy_temp*cx[neg]*cy[neg]));
            //Perform BB for the Non-equilibrium parts
            f[xCoor][iY][pos]=f[xCoor][iY][neg]+(eqpos-eqneg);
        }

        //Calculate the excess of momenta
        double momx=0.0;
        double momy=0.0;

        for(int iPop=0;iPop<9;iPop++)
        {
			momx+=f[xCoor][iY][iPop]*cx[iPop];
			momy+=f[xCoor][iY][iPop]*cy[iPop];
		}

        momx=(rho_temp*ux_temp-momx)/diagonal.size();
        momy=(rho_temp*uy_temp-momy)/diagonal.size();
        //std::cout<<f[xCoor][iY][2]-f[xCoor][iY][4]<<" compare with "<<2*momy<<"\n";


        //Add the correction to the diagonal populations
        for(int iPop=0;iPop<diagonal.size();iPop++)
        {
            f[xCoor][iY][diagonal[iPop]] +=
               (!normx)*cx[diagonal[iPop]]*momx
               +(!normy)*cy[diagonal[iPop]]*momy;
        }

		double feqeq[9];
		for(int iPop=0;iPop<9; iPop++)
		{
// 					feqeq[iPop]=weights[iPop]*dense*(1.0+3.0*cx[iPop]*uxeq+3.0*cy[iPop]*uyeq+
// 					4.5*(cx[iPop]*cx[iPop]-1.0/3.0)*uxeq*uxeq+9.0*cx[iPop]*cy[iPop]*uxeq*uyeq+4.5*(cy[iPop]*cy[iPop]-1.0/3.0)*uyeq*uyeq+
// 					81.0/4.0*(cx[iPop]*cx[iPop]*cy[iPop]*cy[iPop]-1.0/3.0*cx[iPop]*cx[iPop]-1.0/3.0*cy[iPop]*cy[iPop]+1.0/9.0)*uxeq*uxeq*uyeq*uyeq+
// 					27.0/2.0*(cx[iPop]*cx[iPop]*cy[iPop]-1.0/3.0*cy[iPop])*uxeq*uxeq*uyeq+
// 					27.0/2.0*(cy[iPop]*cy[iPop]*cx[iPop]-1.0/3.0*cx[iPop])*uyeq*uyeq*uxeq);

			feqeq[iPop]=weights[iPop]*(rho_temp+3.0*cx[iPop]*ux_temp+3.0*cy[iPop]*uy_temp+
                        4.5*(cx[iPop]*cx[iPop]-1.0/3.0)*ux_temp*ux_temp
                        +9.0*cx[iPop]*cy[iPop]*ux_temp*uy_temp
                        +4.5*(cy[iPop]*cy[iPop]-1.0/3.0)*uy_temp*uy_temp);

			f2[xCoor][iY][iPop]=f[xCoor][iY][iPop]*(1.0-omega)+omega*feqeq[iPop];
        }




    }

}

void pressure_populations(int xCoor, int normx, int normy,double rho_pressure)
{
    std::vector<int> unknown;
    std::vector<int> known;
    std::vector<int> positive;
    std::vector<int> negative;
    std::vector<int> neutral;
    std::vector<int> diagonal;

    for(int iCoor=0;iCoor<9;iCoor++)
    {
        //Determination of the projection
        int dot_product=cx[iCoor]*normx+cy[iCoor]*normy;

        if (dot_product>0)
        {
            unknown.push_back(iCoor);
            positive.push_back(iCoor);
            if (! ((cx[iCoor]==normx)&&(cy[iCoor]==normy)) )
            {
                diagonal.push_back(iCoor);
            }
        }
        else
        {
            known.push_back(iCoor);
            if (dot_product<0)
                negative.push_back(iCoor);
            else
                neutral.push_back(iCoor);
        }
    }

//    for(int iCoor=0;iCoor<neutral.size();iCoor++)
//    {
//        std::cout<<"Neutral populations="<<neutral[iCoor]<<"\n";
//    }
//    std::cout<<"\n";

    //Update macroscopic fields
    for (int iY=1;iY<NY-1;iY++)
    {
        rho[xCoor][iY]=rho_pressure;

        double denseminus=0.0;
        double denseneutral=0.0;

        for(int iCount=0;iCount<negative.size();iCount++)
            denseminus+=f[xCoor][iY][negative[iCount]];

        for(int iCount=0;iCount<neutral.size();iCount++)
            denseneutral+=f[xCoor][iY][neutral[iCount]];

        double norma=sqrt(normx*normx+normy*normy);

        double velocity_perp=(-2.0*denseminus-denseneutral+rho_pressure)/rho_pressure;

        ux[xCoor][iY]=velocity_perp*double(normx)/norma;
        uy[xCoor][iY]=velocity_perp*double(normy)/norma;

        double rho_temp=rho[xCoor][iY];
        double ux_temp=ux[xCoor][iY];
        double uy_temp=uy[xCoor][iY];

        //Perform non-equilibrium BB for the unknown populations
        for(int iCount=0;iCount<unknown.size();iCount++)
        {
            int pos=unknown[iCount];


            double eqpos=weights[pos]*(rho_temp + 3.0*rho_temp*(cx[pos]*ux_temp+cy[pos]*uy_temp)
                            +4.5*rho_temp*((cx[pos]*cx[pos]-1.0/3.0)*ux_temp*ux_temp
                                             +(cy[pos]*cy[pos]-1.0/3.0)*uy_temp*uy_temp
                                             +2.0*ux_temp*uy_temp*cx[pos]*cy[pos]));

            int neg=compliment[unknown[iCount]];
            double eqneg=weights[neg]*(rho_temp + 3.0*rho_temp*(cx[neg]*ux_temp+cy[neg]*uy_temp)
                            +4.5*rho_temp*((cx[neg]*cx[neg]-1.0/3.0)*ux_temp*ux_temp
                                             +(cy[neg]*cy[neg]-1.0/3.0)*uy_temp*uy_temp
                                             +2.0*ux_temp*uy_temp*cx[neg]*cy[neg]));
            //Perform BB for the Non-equilibrium parts
            f[xCoor][iY][pos]=f[xCoor][iY][neg]+(eqpos-eqneg);
        }

        //Calculate the excess of momenta
        double momx=0.0;
        double momy=0.0;

        for(int iPop=0;iPop<9;iPop++)
        {
			momx+=f[xCoor][iY][iPop]*cx[iPop];
			momy+=f[xCoor][iY][iPop]*cy[iPop];
		}

        momx=(rho_temp*ux_temp-momx)/diagonal.size();
        momy=(rho_temp*uy_temp-momy)/diagonal.size();
        //std::cout<<f[xCoor][iY][2]-f[xCoor][iY][4]<<" compare with "<<2*momy<<"\n";


        //Add the correction to the diagonal populations
        for(int iPop=0;iPop<diagonal.size();iPop++)
        {
            f[xCoor][iY][diagonal[iPop]] +=
               (!normx)*cx[diagonal[iPop]]*momx
               +(!normy)*cy[diagonal[iPop]]*momy;
        }


    }

}




void collide_stream_bulk()
{
    for(int iX=1;iX<NX-1;iX++)
        for(int iY=1;iY<NY-1;iY++)
		{
			//Construction equilibrium
			rho[iX][iY]=0.0;
			ux[iX][iY]=0.0;
			uy[iX][iY]=0.0;
			for(int iPop=0;iPop<9;iPop++)
			{
				rho[iX][iY]+=f[iX][iY][iPop];
				ux[iX][iY]+=f[iX][iY][iPop]*cx[iPop];
				uy[iX][iY]+=f[iX][iY][iPop]*cy[iPop];
			}

			ux[iX][iY]=ux[iX][iY]/rho[iX][iY];
			uy[iX][iY]=uy[iX][iY]/rho[iX][iY];

			double dense=rho[iX][iY];
			double uxeq=ux[iX][iY];
			double uyeq=uy[iX][iY];

			//Construction of the equilibrium moments
			double eq[9];
			eq[0]=dense;
			eq[1]=sqrt(3.0)*dense*uxeq;
			eq[2]=sqrt(3.0)*dense*uyeq;
			eq[3]=3.0/sqrt(2.0)*dense*uxeq*uxeq;
			eq[4]=3.0*dense*uxeq*uyeq;
			eq[5]=3.0/sqrt(2.0)*dense*uyeq*uyeq;
			eq[6]=4.5*dense*uxeq*uxeq*uyeq*uyeq;
			eq[7]=3.0*sqrt(1.5)*dense*uxeq*uyeq*uyeq;
			eq[8]=3.0*sqrt(1.5)*dense*uxeq*uxeq*uyeq;

			double feqeq[9];
			for(int iPop=0;iPop<9; iPop++)
			{
// 					feqeq[iPop]=weights[iPop]*dense*(1.0+3.0*cx[iPop]*uxeq+3.0*cy[iPop]*uyeq+
// 					4.5*(cx[iPop]*cx[iPop]-1.0/3.0)*uxeq*uxeq+9.0*cx[iPop]*cy[iPop]*uxeq*uyeq+4.5*(cy[iPop]*cy[iPop]-1.0/3.0)*uyeq*uyeq+
// 					81.0/4.0*(cx[iPop]*cx[iPop]*cy[iPop]*cy[iPop]-1.0/3.0*cx[iPop]*cx[iPop]-1.0/3.0*cy[iPop]*cy[iPop]+1.0/9.0)*uxeq*uxeq*uyeq*uyeq+
// 					27.0/2.0*(cx[iPop]*cx[iPop]*cy[iPop]-1.0/3.0*cy[iPop])*uxeq*uxeq*uyeq+
// 					27.0/2.0*(cy[iPop]*cy[iPop]*cx[iPop]-1.0/3.0*cx[iPop])*uyeq*uyeq*uxeq);

					feqeq[iPop]=weights[iPop]*(dense+3.0*cx[iPop]*uxeq+3.0*cy[iPop]*uyeq+
						4.5*(cx[iPop]*cx[iPop]-1.0/3.0)*uxeq*uxeq+9.0*cx[iPop]*cy[iPop]*uxeq*uyeq+4.5*(cy[iPop]*cy[iPop]-1.0/3.0)*uyeq*uyeq);
            }

			double add[9];
			double addit;
			for(int iPop=0;iPop < 9; iPop++)
			{
				add[iPop]=0.0;
				for(int k=0; k < 9; k++)
					add[iPop]=add[iPop]+M[iPop][k]*(-f[iX][iY][k]); //+feqeq[k]); //+node.popeq[k]);
				add[iPop]+=eq[iPop];
			}

			for(int k=0; k < 9; k++)
			{
				addit=0.0;
				for(int m=0; m < 9; m++)
					addit=addit+omegamat[m]*M[m][k]*add[m];
				//f2[iX][iY][k]=f[iX][iY][k]+weights[k]*addit; //+feqforce;
				f2[iX][iY][k]=f[iX][iY][k]*(1.0-omega)+omega*feqeq[k]; //+feqforce;
			}


		}

}

void update_bounce_back()
{
	//BB nodes density and velocity specification
	for(int iX=0;iX<NX;iX++)
	{
		int iXtop=(iX+1+NX)%NX;
		int iXbottom=(iX-1+NX)%NX;

		f2[iX][0][2]=f2[iX][1][4];
		f2[iX][0][5]=f2[iXtop][1][7];
		f2[iX][0][6]=f2[iXbottom][1][8];

		f2[iX][NY-1][4]=f2[iX][NY-2][2];
		f2[iX][NY-1][7]=f2[iXbottom][NY-2][5];
		f2[iX][NY-1][8]=f2[iXtop][NY-2][6];

		rho[iX][0]=1.0;
		rho[iX][NY-1]=1.0;
		ux[iX][0]=0.0;ux[iX][NY-1]=0.0;
		uy[iX][0]=0.0;uy[iX][NY-1]=0.0;
	}

}

void collide_pressure()
{

    for(int iY=1;iY<NY-1;iY++)
    {
		double feqeq[9];
        double rho_temp=0.0;
        double ux_temp=0.0;
		double uy_temp=0.0;
		for(int iPop=0;iPop<9;iPop++)
		{
			rho_temp+=f[0][iY][iPop];
			ux_temp+=f[0][iY][iPop]*cx[iPop];
			uy_temp+=f[0][iY][iPop]*cy[iPop];
		}

		ux_temp=ux_temp/rho_temp;
		uy_temp=uy_temp/rho_temp;

		for(int iPop=0;iPop<9; iPop++)
		{
// 					feqeq[iPop]=weights[iPop]*dense*(1.0+3.0*cx[iPop]*uxeq+3.0*cy[iPop]*uyeq+
// 					4.5*(cx[iPop]*cx[iPop]-1.0/3.0)*uxeq*uxeq+9.0*cx[iPop]*cy[iPop]*uxeq*uyeq+4.5*(cy[iPop]*cy[iPop]-1.0/3.0)*uyeq*uyeq+
// 					81.0/4.0*(cx[iPop]*cx[iPop]*cy[iPop]*cy[iPop]-1.0/3.0*cx[iPop]*cx[iPop]-1.0/3.0*cy[iPop]*cy[iPop]+1.0/9.0)*uxeq*uxeq*uyeq*uyeq+
// 					27.0/2.0*(cx[iPop]*cx[iPop]*cy[iPop]-1.0/3.0*cy[iPop])*uxeq*uxeq*uyeq+
// 					27.0/2.0*(cy[iPop]*cy[iPop]*cx[iPop]-1.0/3.0*cx[iPop])*uyeq*uyeq*uxeq);

			feqeq[iPop]=weights[iPop]*(rho_temp+3.0*cx[iPop]*ux_temp+3.0*cy[iPop]*uy_temp+
                        4.5*(cx[iPop]*cx[iPop]-1.0/3.0)*ux_temp*ux_temp
                        +9.0*cx[iPop]*cy[iPop]*ux_temp*uy_temp
                        +4.5*(cy[iPop]*cy[iPop]-1.0/3.0)*uy_temp*uy_temp);

			f2[0][iY][iPop]=f[0][iY][iPop]*(1.0-omega)+omega*feqeq[iPop];
        }

        rho_temp=0.0;
        ux_temp=0.0;
		uy_temp=0.0;
		for(int iPop=0;iPop<9;iPop++)
		{
			rho_temp+=f[NX-1][iY][iPop];
			ux_temp+=f[NX-1][iY][iPop]*cx[iPop];
			uy_temp+=f[NX-1][iY][iPop]*cy[iPop];
		}

		ux_temp=ux_temp/rho_temp;
		uy_temp=uy_temp/rho_temp;

		for(int iPop=0;iPop<9; iPop++)
		{
// 					feqeq[iPop]=weights[iPop]*dense*(1.0+3.0*cx[iPop]*uxeq+3.0*cy[iPop]*uyeq+
// 					4.5*(cx[iPop]*cx[iPop]-1.0/3.0)*uxeq*uxeq+9.0*cx[iPop]*cy[iPop]*uxeq*uyeq+4.5*(cy[iPop]*cy[iPop]-1.0/3.0)*uyeq*uyeq+
// 					81.0/4.0*(cx[iPop]*cx[iPop]*cy[iPop]*cy[iPop]-1.0/3.0*cx[iPop]*cx[iPop]-1.0/3.0*cy[iPop]*cy[iPop]+1.0/9.0)*uxeq*uxeq*uyeq*uyeq+
// 					27.0/2.0*(cx[iPop]*cx[iPop]*cy[iPop]-1.0/3.0*cy[iPop])*uxeq*uxeq*uyeq+
// 					27.0/2.0*(cy[iPop]*cy[iPop]*cx[iPop]-1.0/3.0*cx[iPop])*uyeq*uyeq*uxeq);

			feqeq[iPop]=weights[iPop]*(rho_temp+3.0*cx[iPop]*ux_temp+3.0*cy[iPop]*uy_temp+
                        4.5*(cx[iPop]*cx[iPop]-1.0/3.0)*ux_temp*ux_temp
                        +9.0*cx[iPop]*cy[iPop]*ux_temp*uy_temp
                        +4.5*(cy[iPop]*cy[iPop]-1.0/3.0)*uy_temp*uy_temp);

			f2[NX-1][iY][iPop]=f[NX-1][iY][iPop]*(1.0-omega)+omega*feqeq[iPop];
        }







    }

}

int main(int argc, char* argv[])
{

    matrix_init();
    init();

	//writedensity("dense_init.dat");
	//writevelocity("momentum_init.dat");


	for(int counter=0;counter<=N;counter++)
	{

        general_pressure(0,1,0,rho_inlet);
        general_pressure(NX-1,-1,0,rho_outlet);

        collide_stream_bulk();

        //To use only if general_pressure after streaming
        //collide_pressure();

        update_bounce_back();

		//Streaming
		for(int iX=0;iX<NX;iX++)
			for(int iY=1;iY<NY-1;iY++)
				for(int iPop=0;iPop<9;iPop++)
				{
					int iX2=(iX-cx[iPop]+NX)%NX;
					int iY2=(iY-cy[iPop]+NY)%NY;
					f[iX][iY][iPop]=f2[iX2][iY2][iPop];
				}

        //pressure_populations(0,1,0,rho_inlet);
        //pressure_populations(NX-1,-1,0,rho_outlet);

		//Writing files
		if (counter%NOUTPUT==0)
		{
			std::cout<<counter<<"\n";

			std::stringstream filewritedensity;
 			std::stringstream filewritevelocity;
 			std::stringstream counterconvert;
 			counterconvert<<counter;
 			filewritedensity<<std::fixed;
			filewritevelocity<<std::fixed;

			filewritedensity<<"proba"<<std::string(6-counterconvert.str().size(),'0')<<counter;
			filewritevelocity<<"phase"<<std::string(6-counterconvert.str().size(),'0')<<counter;

 			writedensity(filewritedensity.str());
			writevelocity(filewritevelocity.str());
		}


	}

	return 0;
}
