//
//  BasicCluster.hpp
//
//  Created by Jeremi Niedziela on 12/06/2018.
//

#ifndef BasicCluster_hpp
#define BasicCluster_hpp

#include "Hexel.hpp"

#include <vector>

/// Class representing cluster of hexels
class BasicCluster{
public:
  /// Default constructor taking as an input basic parameters of a cluster and a vector of hexels.
  /// Automatically calculates eta and phi from XYZ coordinates
  BasicCluster(double _energy=-1, double _x=0, double _y=0, double _z=0,
               std::vector<std::shared_ptr<Hexel>> _thisCluster=std::vector<std::shared_ptr<Hexel>>(0));
  
  /// Default constructor taking as an input basic parameters of a cluster and a vector of hexels.
  BasicCluster(double _energy=-1, double _x=0, double _y=0, double _z=0, double _eta=0, double _radius=0);
  
  ~BasicCluster();

  /// Returns pointer to collection of hexels associated with this cluster
  inline std::vector<std::shared_ptr<Hexel>> GetHexelsInThisCluster(){return thisCluster;}

  /// Returns pseudorapidity of this cluster
  inline double GetEta() const {return eta;}
  
  /// Returns energy of this cluster
  inline double GetEnergy() const {return energy;}

  /// Returns X coordinate of this cluster
  inline double GetX() const {return x;}
  
  /// Returns Y coordinate of this cluster
  inline double GetY() const {return y;}
  
  /// Returns maximum radius of the cluster
  inline double GetRadius() const {return radius;}
  
  bool operator=(BasicCluster &b);
private:
  double energy;///< Energy of the cluster (GeV)
  double x;     ///< X coordinate of the cluster
  double y;     ///< Y coordinate of the cluster
  double z;     ///< Z coordinate of the cluster
  double eta;   ///< Pseudorapidity of the cluster
  double phi;   ///< Polar angle of the cluster
  double radius;

  std::vector<std::shared_ptr<Hexel>> thisCluster;     ///< Hexels belonging to this cluster
};

#endif /* BasicCluster_hpp */
