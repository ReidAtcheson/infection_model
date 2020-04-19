#include <iostream>
#include <map>
#include <fstream>
#include <random>
#include <vector>
#include <cmath>
#include <omp.h>

std::map<std::string,std::string> parameter_file(std::string filename);

int main(int argc,char** argv){
  if(argc<=1){
    std::cerr<<"Usage: ./main parameter_file"<<std::endl;
    return 1;
  }
  auto parameters=parameter_file(argv[1]);

  double pi = 4.0*std::atan(1.0);

  /*Number of initial infections
   * (randomly chosen).*/
  int ninit=std::stoi(parameters["ninit"]);

  /*Output files for printing nice
   * scatter plots in Python
   * (don't turn on if big population).*/
  bool output_nodes=parameters["output_nodes"]==std::string("True")?true:false;
  /*Max number of 
   * simulation steps to output.*/
  int max_print_nodes=std::stoi(parameters["max_print_nodes"]);


  /*Number of people in simulation.*/
  int npeople=std::stoi(parameters["npeople"]);

  /*Grid bounds.*/
  double xleft=std::stof(parameters["xleft"]);
  double xright=std::stof(parameters["xright"]);
  double ybottom=std::stof(parameters["ybottom"]);
  double ytop=std::stof(parameters["ytop"]);



  /*Predicate to determine if
   * a point is inside the above
   * defined box.*/
  auto contained = [&](double x,double y){return x>=xleft && x<=xright && y>=ybottom && y<=ytop;};

  /*
   *  How many simulation periods
   *  (time-steps) a person is infectious for.
   */
  int infectious_period=std::stoi(parameters["infectious_period"]);

  /*
   * Predicate to determine if someone is infectious
   */
  auto infectious =  [&](int days){return days>0 && days<=infectious_period;};
  /*
   * Predicate to determine if someone is susceptible
   */
  auto susceptible = [&](int days){return days==0;};




  /*Distance to move each time-step.*/
  double dx=std::stof(parameters["dx"]);
  double dy=std::stof(parameters["dy"]);

  /*Number of steps to run simulation for.*/
  int nsteps=std::stoi(parameters["nsteps"]);

  /*If two people get within this radius they trigger infection chance
   * (if one is infected) .*/
  double infection_radius=std::stof(parameters["infection_radius"]);

  /*Probability of infected person infecting a non-infected within radius.*/
  double probability=std::stof(parameters["probability"]);

  /*Probability of importing a new case.*/
  double import_probability=std::stof(parameters["import_probability"]);



  /*Set up values for population.*/
  std::vector<int> infected_previous(npeople,0);
  std::vector<int> infected_current(npeople,0);
  std::vector<double> posx(npeople,0.0);
  std::vector<double> posy(npeople,0.0);
  std::vector<int> immune_windows(npeople,0);


  /*
   * Try to distribute population evenly 
   * over square.
   */
  std::mt19937 ngen (23084);
  std::uniform_real_distribution<double> ux(xleft,xright);
  std::uniform_real_distribution<double> uy(ybottom,ytop);
  for(int i=0;i<npeople;i++){
    posx[i]=ux(ngen);
    posy[i]=uy(ngen);
  }

  /*Turn off this code for now because not clear how to best
   * represent results over larger time-scales when
   * reinfections occur (do we double count reinfections?
   * do we subtract recovered from total infections and add
   * it back again when reinfected)*/
#ifdef __IMMUNE_WINDOW
  /*Give everybody an immunity window, after which they become susceptible again.*/
  int mean_immune_window=1500;
  double clustering=0.8;
  int n=int(mean_immune_window/clustering);
  std::binomial_distribution<int> immune_window(n,clustering);
  for(int i=0;i<npeople;i++){
    immune_windows[i]=immune_window(ngen);
  }
#endif
  



  std::uniform_int_distribution<int> unlucky(0,npeople);
  /*Make a randomly chosen set of people infected at the beginning.*/
  for(int i=0;i<ninit;i++){
    infected_previous[unlucky(ngen)]=1;
  }
  //infected_current[0]=0;

  std::vector<int> ninfections;

#pragma omp parallel
  {
    int tid=omp_get_thread_num();
    /*Set up RNG for random walks and infection challenges.*/
    std::mt19937 gen (tid);
    /*A random angle for determining direction of random walk.*/
    std::uniform_real_distribution<double> uniform(-pi,pi);
    /*Chance that an infected individual infects a susceptible individual
     * when inside the infection radius.*/
    std::bernoulli_distribution bernouli(probability);
    /*Chance that a case gets imported.*/
    std::bernoulli_distribution import(import_probability);
    /*Pick an unlucky person to infect when an imported case happens.*/
    std::uniform_int_distribution<int> unlucky_import(0,npeople);
  

    for(int i=0;i<nsteps;i++){

      if(tid==0){
          /*Check to see if we import a case.*/
          if(import(gen)){
            int id=unlucky_import(gen);
            infected_previous[id]=1;
          }

          int tmp=0;
          /*Get current number of infections.*/
          ninfections.push_back(0);
          for(int d : infected_current){
            if(d>0) { 
              ninfections[ninfections.size()-1]+=1;
              tmp+=1;
            }
          }
          std::cout<<tmp<<std::endl;
      }
#pragma omp barrier

      /*Dump data to files for postprocessing.*/
      if(tid==0 && output_nodes && i<max_print_nodes){
        std::ofstream inf_xnodes(std::string("inf_x_")+std::to_string(i)+std::string(".dat"),std::ofstream::binary);
        std::ofstream inf_ynodes(std::string("inf_y_")+std::to_string(i)+std::string(".dat"),std::ofstream::binary);
        std::ofstream xnodes(std::string("x_")+std::to_string(i)+std::string(".dat"),std::ofstream::binary);
        std::ofstream ynodes(std::string("y_")+std::to_string(i)+std::string(".dat"),std::ofstream::binary);
        std::ofstream infectedf(std::string("inf_")+std::to_string(i)+std::string(".dat"),std::ofstream::binary);
        xnodes.write((char*)posx.data(),posx.size()*sizeof(double));
        ynodes.write((char*)posy.data(),posy.size()*sizeof(double));
        infectedf.write((char*)infected_previous.data(),infected_previous.size()*sizeof(int));

        std::vector<double> inf_x;
        std::vector<double> inf_y;
        for(size_t k=0;k<infected_previous.size();k++){
          if(infectious(infected_previous[k])){
            inf_x.push_back(posx[k]);
            inf_y.push_back(posy[k]);
          }
        }
        inf_xnodes.write((char*)inf_x.data(),inf_x.size()*sizeof(double));
        inf_ynodes.write((char*)inf_y.data(),inf_y.size()*sizeof(double));
      }



      /*Advance positions of random walks.*/
#pragma omp for schedule(dynamic)
      for(int j=0;j<npeople;j++){
        double x=posx[j]+dx*std::cos(uniform(gen));
        double y=posy[j]+dy*std::sin(uniform(gen));
        if(contained(x,y)){
          posx[j]=x;
          posy[j]=y;
        }
      }
#pragma omp for schedule(dynamic)
      for(int j=0;j<npeople;j++){
        if(infectious(infected_previous[j])){
          for(int k=0;k<npeople;k++){
            /*If two people get in the infection radius run the
             * infection logic.*/
            double dist=std::hypot(posx[j]-posx[k],posy[j]-posy[k]);
            if(dist<infection_radius){
              if(bernouli(gen) && susceptible(infected_previous[k])){
#pragma omp atomic 
                infected_current[k]+=1;
              }
            }
          }
        }
      }


      /*Increment infection days for infected population.*/
#pragma omp for schedule(dynamic)
      for(int j=0;j<npeople;j++){
        if(!(susceptible(infected_previous[j]))){
          infected_current[j]+=1;
        }
      }


      /*See above comments on __IMMUNE_WINDOW code path
       * to see why commented out. Will not use
       * for now.*/
#ifdef __IMMUNE_WINDOW
      /*Check how long someone was immune for, if
       * it passes their immunity window they become
       * susceptible again.*/
#pragma omp for schedule(dynamic)
      for(int j=0;j<npeople;j++){
        if(infected_previous[j]>immune_windows[j]){
          infected_current[j]=0;
        }
      }
#endif



      if(tid==0){
       std::copy(infected_current.begin(),infected_current.end(),infected_previous.begin());
      }
   #pragma omp barrier
    }

  }

  std::ofstream out("ninfections.dat",std::ofstream::binary);
  out.write((char*)ninfections.data(),ninfections.size()*sizeof(int));






  return 0;
}


std::map<std::string,std::string> parameter_file(std::string filename){
  std::map<std::string,std::string> out;
  std::ifstream f(filename);
  std::string line;
  while(std::getline(f,line)){
    size_t pos=line.find("=");
    std::string left=line.substr(0,pos);
    std::string right=line.substr(pos+1,std::string::npos);
    out[left]=right;
  }
  return out;
}






