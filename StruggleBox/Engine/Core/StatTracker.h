#ifndef STAT_TRACKER_H
#define STAT_TRACKER_H

#include <vector>

class Locator;
class TextManager;
class Renderer;

class StatTracker
{
public:
    StatTracker(Locator& locator);
    ~StatTracker();
    bool IsVisible( void ) { return s_showStats; };

    void ToggleStats();
    void UpdateStats(Renderer* renderer);
    
    // Renderer statistics setters
    void SetRFPS( const double newFPS );
    void SetRFrameDelta( const double newFrameDelta );
    void SetRTime( const double newTime );
    void SetPTime( const double newTime );
    void SetPCollisions( const int newCollisions );
    void SetPManifodlds( const int newManifolds );
    void SetETime( const double newTime );
    void SetENum( const int newEntities );
    void SetRNumTris( const int newNumTris );
    void SetRNumSegs( const int newNumSegs );
    void SetRNumSpheres( const int newNumSpheres );
    void SetRNumLights2D( const int newNumLights2D );
    void SetRNumLights3D( const int newNumLights3D );
    void SetTNumJobs( const int newJobs );
    // Physics simulations stats
//    void SetPBodies( int newBodies );
//    void SetPStaticBodies( int newStaticBodies );
//    void SetPShapes( int newShapes );
//    void SetPStaticShapes( int newStaticShapes );
//    void SetPArbiters( int newArbiters );
//    void SetPPoints( int newPoints );
//    void SetPConstraints( int newConstraints );
//    void SetPKE( double newKE );
private:
    Locator& _locator;

    // Whether StatTracker is visible
    bool s_showStats;
    
    // Text label identifiers
    std::vector<int> labelVect;
    int barGraphInts[100];
    int barGraphInts2[100];
    float barGraphFloats[100];
    float s_pManifoldsArray[100];
    
    int lastBarNum;
    
    // Renderer statistics
    double s_rFPS;
    double s_rFrameDelta;
    double s_rTime;
    double s_pTime;
    int s_pCollisions;
    int s_pManifolds;
    
    double s_eTime;
    int s_eNum;
    
    int s_rNumTris;
    int s_rNumSegs;
    int s_rNumSpheres;
    int s_rNumLights2D;
    int s_rNumLights3D;
    
    int s_tNumJobs;
    
    void DisplayStats();
    void HideStats();
};

#endif /* STAT_TRACKER_H */
