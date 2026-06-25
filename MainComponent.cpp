#include "MainComponent.h"


bool MainComponent::isMidiConfiured() const
{
        auto* configTab = dynamic_cast<ConfigTab*>(tabbedComponent.getTabContentComponent(0));
    if ((configTab->midiOutPort.getText().isNotEmpty() && configTab->midiInPort.getText().isNotEmpty()) || configTab->MidiConfigbox.getText().isNotEmpty() ) {
        }
    return true;
}

//CONSTRUCTORES Y PESTAÑAS
MainComponent::MainComponent() : tabbedComponent(juce::TabbedButtonBar::TabsAtTop) {
    setLookAndFeel(&modernLook);
    setSize(950, 720);

    tabbedComponent.addTab("Runtime", juce::Colours::lightgrey, new RuntimeTab(*this), true);
    tabbedComponent.addTab("Configuration", juce::Colours::lightgrey, new ConfigTab(*this), true);
    addAndMakeVisible(tabbedComponent);

    // Configuración del label de ganancias
    gainLabel.setJustificationType(juce::Justification::topLeft);
    gainLabel.setFont(juce::FontOptions(14.0f));
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(gainLabel);
    updateSourcesLayout();
    
}
MainComponent::~MainComponent() {setLookAndFeel(nullptr);}
void MainComponent::updateGainDisplay() {

    if (auto* runtimeTab = dynamic_cast<RuntimeTab*>(tabbedComponent.getTabContentComponent(0))) {
        juce::String gainsText;

        for (auto* speaker : runtimeTab->sources) {

            // Verificación explícita de NaN
            float displayGain = std::isnan(speaker->gain) ? 0.0f : speaker->gain;
            gainsText += "L" + juce::String(speaker->LoudSpeakerID) + ": " +
                juce::String(displayGain, 2) + "\n";
        }

        gainLabel.setText(gainsText, juce::dontSendNotification);
    }
}
void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
void MainComponent::resized() {
    tabbedComponent.setBounds(getLocalBounds());
    gainLabel.setBounds(getWidth() - 150, 50, 140, 600);
}
void MainComponent::RuntimeTab::paint(juce::Graphics& g) {
    
    g.fillAll(juce::Colours::darkgrey);

    auto center = getLocalBounds().getCentre().toFloat();
    auto radius = juce::jmin(getWidth(), getHeight()) * 0.4f;

    g.setColour(juce::Colours::aliceblue);
    g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 1.5f);

    g.setColour(juce::Colours::antiquewhite);
    g.drawLine(center.x - radius, center.y, center.x + radius, center.y, 1.5f);
    g.drawLine(center.x, center.y - radius, center.x, center.y + radius, 1.5f);

    g.setColour(juce::Colours::white);
    g.drawText("90", center.x + radius + 5, center.y - 10, 30, 20, juce::Justification::left);
    g.drawText("0", center.x - 10, center.y - radius - 20, 30, 20, juce::Justification::centred);
    g.drawText("270", center.x - radius - 35, center.y - 10, 30, 20, juce::Justification::right);
    g.drawText("180", center.x - 10, center.y + radius + 5, 30, 20, juce::Justification::centred);
}
void MainComponent::RuntimeTab::resized() {
    //buttonn reset
    addAndMakeVisible(resetButton);
    resetButton.setBounds(getWidth()-150, getHeight()-40, 150, 30);
	
    

    //loudspeakers layout
    auto center = getLocalBounds().getCentre().toFloat();
    auto radius = juce::jmin(getWidth(), getHeight()) * 0.4f;
    

    //configuracion para posicionar los labels de la vsources
    for (auto* vsource : Vsources) {
        
        auto currentPos = vsource->getBounds().getCentre().toFloat();
        juce::Point<float> direction = currentPos - center;
        float distance = direction.getDistanceFromOrigin();
        if (distance > 0) {
            direction = direction / distance * radius;
            vsource->setCentrePosition(center + direction);
            // Actualizar label de ángulo
            float angleRad = std::atan2(direction.y, direction.x);
            float angleDeg = juce::radiansToDegrees(angleRad)+90.0f;
            if (angleDeg < 0) angleDeg += 360.0f;
            dynamic_cast<VSourceComponent*>(vsource)->updateAngleLabel(angleDeg);
        }
        for (auto* vsource : Vsources) {
            vsource->angleLabel.toFront(true);
        }
    }
    //posicionar label de los altavoces
    for (auto* speaker : sources) {
        auto currentPos = speaker->getBounds().getCentre().toFloat();
        juce::Point<float> direction = currentPos - center;
        float distance = direction.getDistanceFromOrigin();
        if (distance > 0) {
            direction = direction / distance * radius;
            speaker->setCentrePosition(center + direction);
            // Actualizar label de ángulo
            float angleRad = std::atan2(direction.y, direction.x);
            float angleDeg = juce::radiansToDegrees(angleRad);
            if (angleDeg < 0) angleDeg += 360.0f;
            dynamic_cast<SourceComponent*>(speaker)->updateAngleLabel(angleDeg);
        }
        for (auto* source : sources) {
            source->angleLabel.toFront(true);
        }
    }

    //virtual sources layout
    auto Vcenter = getLocalBounds().getCentre().toFloat();
    auto Vradius = juce::jmin(getWidth(), getHeight()) * 0.4f;
    const float Vangle1 = juce::MathConstants<float>::pi * 4.0f / 8.0f;
    if (Vsources.size() >= 0) {
        const float VangleStep = juce::MathConstants<float>::twoPi / Vsources.size();
        for (int i = 0; i < Vsources.size(); ++i) {
            float Vangle = Vangle1 + VangleStep * (i - 1);
            Vsources[i]->setTopLeftPosition(
                Vcenter.x + Vradius * std::cos(Vangle) - 20,
                Vcenter.y + Vradius * std::sin(Vangle) - 20
            );
        }
    }
    owner.updateSpeakerPositions();
}
//FUNCIONALIDAD DE ACTUALIZACION DE FUENTES
void MainComponent::updateSourcesLayout() {
    // Obtenemos punteros a las pestañas
    auto* runtimeTab = dynamic_cast<RuntimeTab*>(tabbedComponent.getTabContentComponent(0));
    auto* configTab = dynamic_cast<ConfigTab*>(tabbedComponent.getTabContentComponent(1));

    // Verificación de seguridad
    if (!runtimeTab || !configTab) return;

    // ==================================================================================
    // PARTE 1: GESTIÓN DE FUENTES VIRTUALES (VSources)
    // ==================================================================================
    // Esta parte se ejecuta siempre para permitir añadir/quitar fuentes dinámicamente
    
    int targetNumVSources = configTab->VirtualSourceInput.getText().getIntValue();
    targetNumVSources = juce::jmax(1, targetNumVSources);

    // Solo modificamos si la cantidad ha cambiado
    if (runtimeTab->Vsources.size() != targetNumVSources) {
        while (runtimeTab->Vsources.size() < targetNumVSources) {
            auto* source = new VSourceComponent(runtimeTab->Vsources.size() + 1);
            runtimeTab->Vsources.add(source);
            runtimeTab->addAndMakeVisible(source);
            
            // Posicionar nuevas fuentes en el centro por defecto para no perderlas
            source->setCentrePosition(runtimeTab->getLocalBounds().getCentre().toFloat());
        }
        while (runtimeTab->Vsources.size() > targetNumVSources) {
            runtimeTab->Vsources.removeLast();
        }
        // Llamamos a resized() solo para las fuentes virtuales si es necesario
        // (Aunque para VSources no suele ser crítico el layout automático)
    }
    // 2. REPOSICIONAMIENTO EQUIDISTANTE (Siempre se ejecuta al cambiar el número)
        auto center = runtimeTab->getLocalBounds().getCentre().toFloat();
        // Usamos un radio más pequeño (ej. 100px) para que las fuentes aparezcan
        // en un anillo interno, diferenciándose de los altavoces
        float vRadius = 280.0f;

        for (int i = 0; i < runtimeTab->Vsources.size(); ++i) {
            // Calcular ángulo: 360 grados / número de fuentes
            float angle = juce::MathConstants<float>::twoPi * i / targetNumVSources;
            
            // IMPORTANTE: Restamos MathConstants<float>::halfPi (90°) si quieres que
            // la primera fuente empiece arriba al centro (12 en punto).
            float adjustedAngle = angle - juce::MathConstants<float>::halfPi;
//esto del setcentre esta raro porque arriba si funciona bien
            runtimeTab->Vsources[i]->::MainComponent::setCentrePosition(
                center.x + vRadius * std::cos(adjustedAngle),
                center.y + vRadius * std::sin(adjustedAngle)
            );
        }

    // Actualizar SIEMPRE la estructura de datos de la matriz de ganancias (Backend)
    // Esto es seguro porque resize() en vectores std conserva los datos existentes.
    {
        const juce::ScopedLock lock(gainslock);
        int numSpeakersCurrent = runtimeTab->sources.size();
        
        if (virtualSourcesGains.size() != targetNumVSources) {
            virtualSourcesGains.resize(targetNumVSources);
            // Asegurar que cada nueva fuente tenga el vector de canales correcto
            for (int i = 0; i < targetNumVSources; ++i) {
                virtualSourcesGains[i].sourceID = i + 1;
                if (virtualSourcesGains[i].gains.size() != numSpeakersCurrent) {
                    virtualSourcesGains[i].gains.resize(numSpeakersCurrent, 0.0f);
                }
            }
        }
    }

    // ==================================================================================
    // PARTE 2: GESTIÓN DE ALTAVOCES (Speakers) - CON PROTECCIÓN
    // ==================================================================================
    
    int targetNumSpeakers = configTab->loudspeakersInput.getText().getIntValue();
    targetNumSpeakers = juce::jmax(2, targetNumSpeakers);

    // [CRÍTICO] AQUÍ ESTÁ EL ARREGLO:
    // Comparamos si la cantidad que ya tienes es igual a la que pide el input.
    // Si tienes 4 y el input dice 4, NO ENTRA, y tus posiciones se mantienen intactas.
    
    if (runtimeTab->sources.size() != targetNumSpeakers)
    {
        // A. Actualizar RuntimeTab (Vista Principal)
        while (runtimeTab->sources.size() < targetNumSpeakers) {
            auto* source = new SourceComponent(runtimeTab->sources.size() + 1);
            runtimeTab->sources.add(source);
            runtimeTab->addAndMakeVisible(source);
        }
        while (runtimeTab->sources.size() > targetNumSpeakers) {
            runtimeTab->sources.removeLast();
        }

        // Solo llamamos a resized() si la cantidad de altavoces cambió.
        // Esto reseteará las posiciones al círculo, PERO solo cuando cambias el setup de hardware.
        runtimeTab->resized();

        // B. Actualizar ConfigTab (Vista Configuración - Espejo)
        while (configTab->configSources.size() < targetNumSpeakers) {
            auto* source = new SourceComponent(configTab->configSources.size() + 1);
            source->setInterceptsMouseClicks(true, false);
            configTab->configSources.add(source);
            configTab->addAndMakeVisible(source);
        }
        while (configTab->configSources.size() > targetNumSpeakers) {
            configTab->configSources.removeLast();
        }
        configTab->resized();
        
        // C. Actualizar tamaño de vectores de ganancia internos para los nuevos altavoces
        {
            const juce::ScopedLock lock(gainslock);
            for (auto& vsData : virtualSourcesGains) {
                // Rellenar con 0.0f si crecen, o recortar si disminuyen
                vsData.gains.resize(targetNumSpeakers, 0.0f);
            }
        }
    }
    // ELSE: Si el número es igual, el código salta este bloque y tus altavoces no se tocan.
}
void SourceComponent::mouseDrag(const juce::MouseEvent& event) {//revisar la resolucion de 1 grado de arrastre
    if (auto* configTab = dynamic_cast<MainComponent::ConfigTab*>(getParentComponent())) {
        auto bounds = configTab->getLocalBounds();
        auto center = bounds.getCentre().toFloat();
        const float radius = bounds.getHeight() * 0.28f;
        const float deadZone = 10.0f; // Zona muerta cerca del centro


        auto mousePos = event.getEventRelativeTo(configTab).position;
        juce::Point<float> mouseDelta = mousePos - center;
        float distance = mouseDelta.getDistanceFromOrigin();

        // Filtramos movimientos dentro de la zona muerta
        if (distance < deadZone) {
            return; // Ignorar movimientos muy cercanos al centro
        }

        // Suavizado controlado solo para ángulos (no para posición)
        static float lastValidAngle = std::atan2(
            getBounds().getCentre().y - center.y,
            getBounds().getCentre().x - center.x
        );

        float targetAngle = std::atan2(mouseDelta.y, mouseDelta.x);
        float angleDiff = targetAngle - lastValidAngle;

        // Normalizar la diferencia de ángulo
        while (angleDiff > juce::MathConstants<float>::pi)
            angleDiff -= juce::MathConstants<float>::twoPi;
        while (angleDiff < -juce::MathConstants<float>::pi)
            angleDiff += juce::MathConstants<float>::twoPi;

        // Suavizado solo angular
        float smoothAngle = lastValidAngle + angleDiff * 0.3f;

        // Posición final

        setCentrePosition({
            center.x + radius * std::cos(smoothAngle),
            center.y + radius * std::sin(smoothAngle)
            });

        // Actualizar para el próximo movimiento
        lastValidAngle = smoothAngle;
        //  ÁNGULO para el label de las fuentes virtuales ---
        float angleRad = std::atan2(mouseDelta.y, mouseDelta.x);
        float angleDeg1 = juce::radiansToDegrees(angleRad);
        float angleDeg = 90.0f + angleDeg1; // Tu ajuste de 90°
        if (angleDeg < 0) angleDeg += 360.0f;
        //  ÁNGULO para el label de las fuentes virtuales.
            updateAngleLabel(angleDeg);
            MainComponent& mainComp = configTab->getOwner();
            mainComp.updateSpeakerPositions();
            mainComp.VBAPImplementation(center, radius,smoothAngle, event);
            
    }
}
void VSourceComponent::mouseDrag(const juce::MouseEvent& event) {

    
    if (auto* runtimeTab = dynamic_cast<MainComponent::RuntimeTab*>(getParentComponent())) {
        auto bounds = runtimeTab->getLocalBounds();
        auto center = bounds.getCentre().toFloat();
        const float radius = bounds.getHeight() * 0.4f;
        const float deadZone = 10.0f; // Zona muerta cerca del centro


        auto mousePos = event.getEventRelativeTo(runtimeTab).position;
        juce::Point<float> mouseDelta = mousePos - center;
        float distance = mouseDelta.getDistanceFromOrigin();

        // Filtramos movimientos dentro de la zona muerta
        if (distance < deadZone) {
            return; // Ignorar movimientos muy cercanos al centro
        }

        // Suavizado controlado solo para ángulos (no para posición)
        static float lastValidAngle = std::atan2(
            getBounds().getCentre().y - center.y,
            getBounds().getCentre().x - center.x
        );

        float targetAngle = std::atan2(mouseDelta.y, mouseDelta.x);
        float angleDiff = targetAngle - lastValidAngle;

        // Normalizar la diferencia de ángulo
        while (angleDiff > juce::MathConstants<float>::pi)
            angleDiff -= juce::MathConstants<float>::twoPi;
        while (angleDiff < -juce::MathConstants<float>::pi)
            angleDiff += juce::MathConstants<float>::twoPi;

        // Suavizado solo angular (0.3 es el factor de suavizado, ajustable)
        float smoothAngle = lastValidAngle + angleDiff * 0.3f;

        // Posición final (sin suavizado radial)

        setCentrePosition(mousePos);

        // Actualizar para el próximo movimiento
        lastValidAngle = smoothAngle;

        //  ÁNGULO para el label de las fuentes virtuales ---
        float angleRad = std::atan2(mouseDelta.y, mouseDelta.x);
        float angleDeg1 = juce::radiansToDegrees(angleRad);
        float angleDeg = 90.0f + angleDeg1; // Tu ajuste de 90°
        if (angleDeg < 0) angleDeg += 360.0f;
		// ------------------------------------ problema de latencia al arrastrar fuentes virtuales ----
        updateAngleLabel(angleDeg);
        static float lastSentAngle = -361.0f;
        // Solo procesar VBAP y enviar MIDI si el cambio en grados es >= 1 grado
        if (std::abs(angleDeg - lastSentAngle) >= 1.0f ) {
            MainComponent& mainComp = runtimeTab->getOwner();
            if (VSourceID>32){
                     juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"The loaded configuration it only contains 32 channels, please verify the number of Virtual sources","") ;
                return;
            }
            mainComp.VBAPImplementation(center, radius, smoothAngle, event);
            lastSentAngle = angleDeg;
        }
    }
}
// función para que se actualice en el runtime la posicion del config
void MainComponent::updateSpeakerPositions() {
    if (auto* configTab = dynamic_cast<ConfigTab*>(tabbedComponent.getTabContentComponent(1))) {
        if (auto* runtimeTab = dynamic_cast<RuntimeTab*>(tabbedComponent.getTabContentComponent(0))) {
            auto configCenter = configTab->getLocalBounds().getCentre().toFloat();
            auto runtimeCenter = runtimeTab->getLocalBounds().getCentre().toFloat();
            float runtimeRadius = runtimeTab->getHeight() * 0.4f;

            for (int i = 0; i < configTab->configSources.size() && i < runtimeTab->sources.size(); ++i) {
                // Calcular ángulo relativo en ConfigTab
                auto configPos = configTab->configSources[i]->getBounds().getCentre().toFloat();
                juce::Point<float> configDelta = configPos - configCenter;
                float angle = std::atan2(configDelta.y, configDelta.x);

                // Aplicar mismo ángulo con radio de RuntimeTab
                runtimeTab->sources[i]->setCentrePosition({
                    runtimeCenter.x + runtimeRadius * std::cos(angle),
                    runtimeCenter.y + runtimeRadius * std::sin(angle)
                    });
                
                // ACTUALIZAR LABEL DE ÁNGULO EN RUNTIMETAB
                                float angleDeg1 = juce::radiansToDegrees(angle);
                                float angleDeg = 90.0f + angleDeg1;
                                if (angleDeg < 0) angleDeg += 360.0f;
                                runtimeTab->sources[i]->updateAngleLabel(angleDeg);
            }
            runtimeTab->repaint();
            
        }
    }
}
//VBAP
void MainComponent::updateVirtualSourceGain(int vsourceID, int speakerID, float gain) {
    
    const juce::ScopedLock lock(gainslock);
    if (vsourceID >= 0 && vsourceID < virtualSourcesGains.size() &&
        speakerID >= 0 && speakerID < virtualSourcesGains[vsourceID].gains.size()) {
        virtualSourcesGains[vsourceID].gains[speakerID] = gain;
    }
    
}
void MainComponent::VBAPImplementation(juce::Point<float> center, float radius, float angle, const juce::MouseEvent& event) {
    using namespace Eigen;
	// Verificar que la configuración esté cargada
    if (MixConfigs.empty()) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Configuration Not Loaded",            
            "Go to Configuration tab and select:\n"
            "'Yamaha LS9 config' or 'Load other config'",
            "OK");
        return;
    }

    if (auto* runtimeTab = dynamic_cast<RuntimeTab*>(tabbedComponent.getTabContentComponent(0))) {
        // 1. Validación básica
        if (runtimeTab->sources.size() < 2) {
            // Resetear todas las ganancias a 0 si hay menos de 2 altavoces
            for (auto* speaker : runtimeTab->sources) {
                speaker->gain = 0.0f;
            }
            updateGainDisplay();
            return;
        }
        // 2. Definir vectores base con validación
        std::vector<Vector2f> baseVectors;
        for (auto* speaker : runtimeTab->sources) {
            
            juce::Point<float> pos = speaker->getBounds().getCentre().toFloat();
            juce::Point<float> diff = pos - center;
            Vector2f vec(diff.x, diff.y);
            float norm = vec.norm();
            // Solo incluir vectores válidos
            if (norm > 0.001f) { // Umbral pequeño para evitar divisiones por cero
                vec.normalize();
                baseVectors.push_back(vec);
            }
        }
        // 3. Verificar suficientes vectores válidos
        if (baseVectors.size() < 2) {
            updateGainDisplay();
            return;
        }
        // 4. Selección de pares con verificación de matriz invertible
        Vector2f targetDir(std::cos(angle), std::sin(angle));
        int bestPair[2] = { 0, 1 };
        float bestDot = -1.0f;
        bool validPairFound = false;
        for (size_t i = 0; i < baseVectors.size(); ++i) {
            for (size_t j = i + 1; j < baseVectors.size(); ++j) {
                Matrix2f G;
                G.col(0) = baseVectors[i];
                G.col(1) = baseVectors[j];
                // Verificar si la matriz es invertible
                if (std::abs(G.determinant()) > 0.001f) { // Umbral para considerar invertible
                    float dot1 = baseVectors[i].dot(targetDir);
                    float dot2 = baseVectors[j].dot(targetDir);
                    float combinedDot = dot1 + dot2;
                    if (combinedDot > bestDot) {
                        bestDot = combinedDot;
                        bestPair[0] = static_cast<int>(i);
                        bestPair[1] = static_cast<int>(j);
                        validPairFound = true;
                    }
                }
            }
        }
        // 5. Cálculo de ganancias con protección
        if (validPairFound) {
            Matrix2f G;
            G.col(0) = baseVectors[bestPair[0]];
            G.col(1) = baseVectors[bestPair[1]];
            Vector2f gains = G.inverse() * targetDir;
            // Normalización segura
            float gainNorm = gains.norm();
            if (gainNorm > 0.001f) {
                gains *= (1.0f / gainNorm);
            }
            else {
                gains = Vector2f::Zero();
                
            }
            // 6. Aplicar ganancias
            for (int i = 0; i < runtimeTab->sources.size(); ++i) {
                float newGain = 0.0f;
                if (i == bestPair[0]) newGain = juce::jlimit(0.0f, 1.0f, gains[0]);
                else if (i == bestPair[1]) newGain = juce::jlimit(0.0f, 1.0f, gains[1]);

                // Crossfade suave (Para este caso, se queito por latencia en MIDI)
                runtimeTab->sources[i]->gain = juce::jlimit(0.0f, 1.0f,
                    runtimeTab->sources[i]->gain + 1.0f * (newGain - runtimeTab->sources[i]->gain));
                runtimeTab->sources[i]->repaint();
            }
        }
        else {
            // Fallback: Resetear ganancias si no se encontró par válido
            for (auto* speaker : runtimeTab->sources) {
                speaker->gain = 0.0f;
            }
        }
        //-------3---------//
        //actualizar las matrices de gannancias para cada fuetne virtual
            // Almacenar ganancias para la fuente virtual activa
        if (auto vsource = dynamic_cast<VSourceComponent*>(event.originalComponent)) {
            const juce::ScopedLock lock(gainslock);
            int vsourceID = vsource->VSourceID - 1;
            if (vsourceID >= 0 && vsourceID < virtualSourcesGains.size()) {
                // Redimensionar el vector de ganancias si es necesario
                if (virtualSourcesGains[vsourceID].gains.size() != runtimeTab->sources.size()) {
                    virtualSourcesGains[vsourceID].gains.resize(runtimeTab->sources.size(), 0.0f);
                }

                for (int i = 0; i < runtimeTab->sources.size(); ++i) {
                    virtualSourcesGains[vsourceID].gains[i] = runtimeTab->sources[i]->gain;
                }
            }
        }
        if (!midiOutput) {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"MIDI output not initialized!","Ok");
           return;
        }
        //envio de las ganancias almacenadas
        if (auto vsource = dynamic_cast<VSourceComponent*>(event.originalComponent)) {
            const juce::ScopedLock lock(gainslock);
            int vsourceID = vsource->VSourceID - 1;

            auto* configTab = dynamic_cast<ConfigTab*>(tabbedComponent.getTabContentComponent(1));
            int midiChannel = configTab->MidiChannelbox.getSelectedId();
            //reconocer los altavoces activos
            for (int i = 0; i < runtimeTab->sources.size(); ++i) {
                //int speakerId = bestPair[i];
                float gain = runtimeTab->sources[i]->gain;
                //if (speakerId >= 0 && speakerId < MixConfigs.size()) {
                    //ENVIO DE MENSAJE POR MIX-CANNAL
                    uint16_t value = MixConfigs[i].channelValues[vsourceID];
                    uint8_t lsb, msb;
                    hexToLSBMSB(juce::String::toHexString(value).paddedLeft('0',4), lsb, msb);
                    float gaindb= juce::Decibels::gainToDecibels(gain);
                   
                int gainmapped=13168*std::pow(juce::MathConstants <double>::euler,0.0381*gaindb);
                //revisar el condicional para cuando se usa otra consola de manera que la ecuacion 
                //corresponda al valor 0 db en esa consola

                  if(gainmapped<=0){
                       gainmapped=0;
                  }
                  
                    uint16_t datagain = static_cast<uint16_t>(gainmapped);
                    //uint16_t datagain = static_cast<uint16_t>(gain*16383);
                    uint8_t lsbgain, msbgain;
                    
                    hexToLSBMSB(juce::String::toHexString(datagain).paddedLeft('0', 4), lsbgain, msbgain);
                    //ENMVIO DEL MENSAJE MIDI
                    sendMidiMessage(midiChannel, 98, lsb);
                    sendMidiMessage(midiChannel, 99, msb);
                    sendMidiMessage(midiChannel, 38, lsbgain);
                    sendMidiMessage(midiChannel, 6, msbgain);
                    DBG("---");
                }
                
            }
        }
        // [4] Mantener la actualización de la UI (existente)
        updateGainDisplay();
    }
    
//MIDI
void MainComponent::loadMidiConfiguration() {
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select  CSV Configuration",
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
        "*.csv",
        true
    );
    fileChooser->launchAsync(juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser) {
            auto file = chooser.getResult();
            if (file.existsAsFile()) {
                //DBG("Archivo seleccionado: " << file.getFullPathName());
                MixConfigs = loadConfigFromCSV(file); // Usar el nuevo parser
                applyMidiConfigToVirtualSources();
                
                // Depuración adicional
                if (MixConfigs.empty()) {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"Error: Mixes not Loaded. Please, verify CSV file.","Ok");
                }
                else {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"Correctly loaded settings for" + juce::String(MixConfigs.size()) + " mixes.","Ok");
                }
            }
            else {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"No file was selected.","Ok");
            }
        });
}

void MainComponent::applyMidiConfigToVirtualSources() {
    const juce::ScopedLock lock(gainslock);
    // Asignación segura con verificación de límites
   size_t numToAssign = juce::jmin(virtualSourcesGains.size(), MixConfigs.size());
    for (int i = 0; i < numToAssign; ++i) {
        if (MixConfigs[i].midiChannel >= 1 && MixConfigs[i].midiChannel <= 16) {
            virtualSourcesGains[i].midiChannel = MixConfigs[i].midiChannel;
        }
    }
    if (virtualSourcesGains.size()>32){
        return;
    }else{
        // Debug: Verificar asignación
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,"MIDI Config Applied. Assigned to " + juce::String(virtualSourcesGains.size()) + " sources","");
    }
}
void MainComponent::sendMidiMessage(int channel, int control, int value) {
    // Asegurar que los valores estén en rangos válidos
    channel = juce::jlimit(0, 16, channel);
    control = juce::jlimit(0, 127, control);
    value = juce::jlimit(0, 127, value);
    juce::MidiMessage message = juce::MidiMessage::controllerEvent(channel, control, value);
    midiOutput->sendMessageNow(message);


    
}
void MainComponent::updateMidiPorts() {}
//BOTON RESET
void MainComponent::RuntimeTab::buttonClicked(juce::Button* button) {
    auto* configTab = dynamic_cast<ConfigTab*>(owner.tabbedComponent.getTabContentComponent(1));
    auto* runtimeTab = dynamic_cast<RuntimeTab*>(owner.tabbedComponent.getTabContentComponent(0));
    int midiChannel = configTab->MidiChannelbox.getSelectedId();
    if (button == &resetButton) {
        if (owner.MixConfigs.empty()) {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                "Configuration Not Loaded",                
                "Please load MIDI configuration file first.",
                "OK");
            return;
        }
    }
    if (button == &resetButton) {
        for (int mixIndex = 0; mixIndex < owner.MixConfigs.size(); ++mixIndex) {
                    for (int speakerIndex = 0; speakerIndex < runtimeTab->sources.size(); ++speakerIndex) {
                        
                        // Verificar que tenemos valores para este mix y altavoz
                        if (mixIndex < owner.MixConfigs.size() &&
                            speakerIndex < owner.MixConfigs[mixIndex].channelValues.size()) {
                            
                            uint16_t value = owner.MixConfigs[mixIndex].channelValues[speakerIndex];
                            uint8_t lsb, msb;
                            hexToLSBMSB(juce::String::toHexString(value).paddedLeft('0', 4), lsb, msb);
                            
                            // Enviar mensaje MIDI para este mix y altavoz específico
                            owner.sendMidiMessage(midiChannel, 98, lsb);
                            owner.sendMidiMessage(midiChannel, 99, msb);
                            owner.sendMidiMessage(midiChannel, 38, 0);  // LSB gain = 0
                            owner.sendMidiMessage(midiChannel, 6, 0);   // MSB gain = 0
                            
                            
                        }
                    }
                }
}
}

