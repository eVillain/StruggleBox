#include "Options.h"
#include "Dictionary.h"

#include "FileUtil.h"

#include "Console.h"
#include "CommandProcessor.h"

static bool s_debugOutput = false;

Options::Options() {
    LoadOptions();
    AddConsoleVars();
}
Options::~Options() {
}

void Options::LoadOptions() {
    bool loaded = false;
    std::string path = FileUtil::GetPath();
    if ( FileUtil::DoesFileExist(path, "Options.plist" ) ) {
        path.append("Options.plist");
        if ( LoadOptionData(path.c_str()) ) {
            loaded = true;
            if ( s_debugOutput ) Console::Print("Loaded options file");
        } else {
            if ( s_debugOutput ) Console::Print("Failed to load options file");
        }
    } else {
        if ( s_debugOutput ) Console::Print("No options file to load");
    }
    if ( !loaded ) {
        ResetToDefaults();
        PrintOpts();
        SaveOptions();
    }
}
void Options::SaveOptions() {
    std::string path = FileUtil::GetPath();
    path.append("Options.plist");
    if ( SaveOptionData(path.c_str()) ) {
        if ( s_debugOutput ) Console::Print("Saved options file");
    } else {
        if ( s_debugOutput ) Console::Print("Failed to save options file");
    }
}
bool Options::SaveOptionData(const char *fileName) {
    Dictionary dict;
    std::map<const std::string, Attribute*>::iterator it;
    for ( it = m_Attributes.begin(); it != m_Attributes.end(); it++ ) {
        if ( it->second->IsType<bool>()) {
            dict.setBoolForKey(it->first.c_str(), it->second->as<bool>());
        } else if ( it->second->IsType<int>()) {
            dict.setIntegerForKey(it->first.c_str(), it->second->as<int>());
        } else if ( it->second->IsType<float>()) {
            dict.setFloatForKey(it->first.c_str(), it->second->as<float>());
        } else if ( it->second->IsType<std::string>()) {
            dict.setStringForKey(it->first.c_str(), it->second->as<std::string>());
        }
    }
    if ( dict.saveRootSubDictToFile(fileName) ) {
        return true;
    }
    return false;
}
// Print out options data values
void Options::PrintOpts() {
    std::map<const std::string, Attribute*>::iterator it;
    for (it = m_Attributes.begin(); it != m_Attributes.end(); it++) {
        printf("Options Key: %s, Data: %s\n", it->first.c_str(), it->second->GetValueString().c_str());
    }
}
void Options::AddConsoleVars() {
    std::map<const std::string, Attribute*>::iterator it;
    for ( it = m_Attributes.begin(); it != m_Attributes.end(); it++ ) {
        if ( it->second->IsType<bool>()) {
            Console::AddVar(it->second->as<bool>(), it->first);
        } else if ( it->second->IsType<int>()) {
            Console::AddVar(it->second->as<int>(), it->first);
        } else if ( it->second->IsType<float>()) {
            Console::AddVar(it->second->as<float>(), it->first);
        } else if ( it->second->IsType<std::string>()) {
            Console::AddVar(it->second->as<std::string>(), it->first);
        }
    }
}
bool Options::LoadOptionData(const char *fileName) {
    Dictionary dict;
    if ( dict.loadRootSubDictFromFile(fileName) ) {
        std::vector<std::string> keys = dict.getAllKeys();
        for ( int i=0; i<keys.size(); i++ ) {
            int keyType = dict.getTypeForKey(keys[i].c_str());
            if ( keyType == DD_Bool ) {
                AddOption( keys[i], dict.getBoolForKey(keys[i].c_str()) );
            } else if ( keyType == DD_Int ) {
                AddOption( keys[i], dict.getIntegerForKey(keys[i].c_str()) );
            } else if ( keyType == DD_Float ) {
                AddOption( keys[i], dict.getFloatForKey(keys[i].c_str()) );
            } else if ( keyType == DD_String ) {
                AddOption( keys[i], dict.getStringForKey(keys[i].c_str()) );
            } else {
                printf("[Options] bad type in dictionary for %s (%i)\n", keys[i].c_str(), keyType);
            }
        }
        return true;
    }
    return false;
}

void Options::ResetToDefaults() {
    // Clear out old options
    m_Attributes.empty();
    
    AddOption("version", "0");
    AddOption("h_multiThreading", true);

    AddOption("r_resolutionX", 1280);
    AddOption("r_resolutionY", 720);

    AddOption("r_useShaders", true);
    AddOption("r_deferred", true);
    AddOption("r_fullScreen", false);
    AddOption("r_debug", false);

    AddOption("r_vSync", true);
    AddOption("r_fsAA", false);
    AddOption("r_fxAA", false);
    
    AddOption("r_lighting2D", true);
    AddOption("r_lighting3D", true);
    AddOption("r_debugLights", false);
    AddOption("r_debugShadows", false);
    AddOption("r_lightRays", false);
    AddOption("r_shadows", false);
    AddOption("r_shadowMultitap", false);
    AddOption("r_shadowNoise", false);
    AddOption("r_renderFog", true);
    AddOption("r_fogDensity", 0.75f);
    AddOption("r_fogHeightFalloff", 0.25f);
    AddOption("r_fogExtinctionFalloff", 20.0f);
    AddOption("r_fogInscatteringFalloff", 20.0f);
    
    AddOption("r_renderMap", false);
    
    AddOption("r_sun", true);
    AddOption("r_sunBlur", false);
    AddOption("r_sunFlare", false);
    
    AddOption("r_renderDOF", false);
    AddOption("r_renderFisheye", false);
    AddOption("r_renderVignette", false);
    AddOption("r_renderCorrectGamma", false);
    AddOption("r_renderToneMap", false);

    AddOption("r_renderFlare", false);
    AddOption("r_flareSamples", 5);
    AddOption("r_flareDispersal", 0.3f);
    AddOption("r_flareHaloWidth", 0.45f);
    AddOption("r_flareChromaDistortionX", 0.01f);
    AddOption("r_flareChromaDistortionY", 0.03f);
    AddOption("r_flareChromaDistortionZ", 0.05f);
    AddOption("r_flareThreshold", 0.75f);
    AddOption("r_flareGain", 1.0f);
    
    AddOption("r_renderSSAO", false);
    AddOption("r_SSAOblur", 0);

    AddOption("r_SSAOtotal_strength", 1.0f);
    AddOption("r_SSAObase", 0.0f);
    AddOption("r_SSAOarea", 0.008f);
    AddOption("r_SSAOfalloff", 0.00001f);
    AddOption("r_SSAOradius", 0.06f);
    
    AddOption("r_renderEdge", false);
    AddOption("r_edgeSobel", false);
    AddOption("r_edgeBrightness", 1.0f);
    AddOption("r_edgeThreshold", 0.05f);
    
    AddOption("r_motionBlur", false);
    AddOption("r_renderWireFrame", false);
    AddOption("r_renderPoint", false);
    AddOption("r_renderParticles", true);
    
    AddOption("r_voxelCulling", true);
    
    AddOption("r_grabCursor", false);

    AddOption("e_worldLabels", false);
    AddOption("e_gridRender", false);
    AddOption("e_gridSnap", false);
    AddOption("e_gridSize", 1.0f);

    AddOption("i_invertLeftStickX", false);
    AddOption("i_invertLeftStickY", false);
    AddOption("i_invertRightStickX", false);
    AddOption("i_invertRightStickY", false);
    
    AddOption("a_playSFX", false);
    AddOption("a_playMusic", false);
    AddOption("a_playMenuSFX", false);
    AddOption("a_volumeSFX", 1.0f);
    AddOption("a_volumeMusic", 1.0f);
    AddOption("a_volumeMenu", 0.3f);

    AddOption("e_gridSize", 1.0f);
    AddOption("e_gridSnap", false);
    
    AddOption("n_networkEnabled", false);
    AddOption("d_physics", false);
}
