//
//  ParticleManager.cpp
//  Ingenium
//
//  Created by The Drudgerist on 01/03/14.
//  Copyright (c) 2014 The Drudgerist. All rights reserved.
//

#include "ParticleManager.h"
#include "HyperVisor.h"
#include "Renderer.h"

int ParticleManager::nextParticleSysID = 0;

ParticleManager::ParticleManager( HyperVisor* hv ) {
    m_hyperVisor = hv;
    paused = false;
}

ParticleManager::~ParticleManager() {
    
}

void ParticleManager::Update( double dt ) {
    if ( paused ) return;
    std::map<int,ParticleSys*>::iterator it;
    for (it=m_systems.begin(); it!=m_systems.end(); it++) {
        it->second->Update(dt);
    }
}

ParticleSys* ParticleManager::AddSystem( const std::string filePath, const std::string fileName ) {
    ParticleSys* sys = new ParticleSys( filePath, fileName );
    int sysID = nextParticleSysID++;
    m_systems[sysID] = sys;
    return sys;
}
int ParticleManager::GetSystemID( ParticleSys *system ) {
    std::map<int, ParticleSys*>::iterator it;
    for (it=m_systems.begin(); it != m_systems.end(); it++) {
        if ( it->second == system ) {
            return it->first;
        }
    }
    return -1;
}
void ParticleManager::RemoveSystem( ParticleSys* system ) {
    std::map<int, ParticleSys*>::iterator it;
    for (it=m_systems.begin(); it != m_systems.end(); it++) {
        if ( it->second == system ) {
            // Found system to delete
            m_systems.erase(it);
            delete system;
            system = NULL;
            return;
        }
    }
}
void ParticleManager::RemoveSystem( const int sysID ) {
    if ( m_systems.find(sysID) != m_systems.end() ) {
        ParticleSys* system = m_systems[sysID];
        m_systems.erase(sysID);
        delete system;
        system = NULL;
        return;
    }
}
void ParticleManager::Draw( Renderer *renderer ) {
    std::map<int,ParticleSys*>::iterator it;
    for (it=m_systems.begin(); it!=m_systems.end(); it++) {
        it->second->Draw(renderer);
    }
}
void ParticleManager::DrawUnlitParticles( Renderer* renderer ) {
    std::map<int,ParticleSys*>::iterator it;
    for (it=m_systems.begin(); it!=m_systems.end(); it++) {
        if ( it->second->lighting == ParticleSysLightOff ) {
            it->second->Draw(renderer);
        }
    }
}
void ParticleManager::DrawLitParticles( Renderer* renderer ) {
    std::map<int,ParticleSys*>::iterator it;
    for (it=m_systems.begin(); it!=m_systems.end(); it++) {
        if ( it->second->lighting == ParticleSysLightOn ) {
            it->second->Draw(renderer);
        }
    }
}
