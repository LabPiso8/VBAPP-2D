#pragma once
#include <JuceHeader.h>
#include <Eigen/Dense>

struct MixConfig {
    int midiChannel;      // Canal MIDI
    juce::StringArray sysexMsgs;
    juce::String mixName;
    std::vector<uint16_t> channelValues;
    juce::StringArray channelSysex;
    int totalMixes = 0;
    int totalChannels = 0;

};
//Actualización  de colores para la GUI
// MainComponent.h
namespace Theme {
    const juce::Colour background  = juce::Colour(0xff111111); // Negro casi puro (Dark Grey)
    const juce::Colour gridLines   = juce::Colour(0xff333333); // Gris oscuro para la rejilla
    const juce::Colour accent      = juce::Colour(0xff00E5FF); // Cian Neón (Tron Style)
    const juce::Colour text        = juce::Colour(0xffEEEEEE); // Blanco hueso
    const juce::Colour speakerOff  = juce::Colour(0xff444444); // Altavoz inactivo
}

static void hexToLSBMSB(const juce::String& hexStr, uint8_t& lsb, uint8_t& msb) {
    uint16_t value = static_cast<uint16_t>(hexStr.getHexValue32()); // Convierte HEX a entero

    // Extraer LSB y MSB
    lsb = static_cast<uint8_t>(value & 0x7F);       // 7 bits menos significativos
    msb = static_cast<uint8_t>((value >> 7) & 0x7F); // 7 bits más significativos
}
//clase para visuales chevres
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel()
    {
        // Configurar colores generales del LookAndFeel
        setColour(juce::ComboBox::backgroundColourId, Theme::background);
        setColour(juce::ComboBox::outlineColourId, Theme::gridLines);
        setColour(juce::ComboBox::arrowColourId, Theme::accent);
        setColour(juce::ComboBox::textColourId, Theme::text);
        
        setColour(juce::TextButton::buttonColourId, Theme::background);
        setColour(juce::TextButton::textColourOffId, Theme::text);
        setColour(juce::TextButton::buttonOnColourId, Theme::accent);
        
        setColour(juce::PopupMenu::backgroundColourId, Theme::background);
        setColour(juce::PopupMenu::textColourId, Theme::text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, Theme::accent.withAlpha(0.3f));
    }

    // 1. DIBUJAR BOTONES (Planos y modernos)
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto cornerSize = 4.0f; // Bordes ligeramente redondeados

        // Color base
        auto baseColour = backgroundColour;
        
        // Si el mouse pasa por encima (Hover), lo hacemos un poco más claro
        if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.2f);
            
        // Si se hace click, se pone del color de acento
        if (shouldDrawButtonAsDown)
            baseColour = Theme::accent.darker(0.2f);

        g.setColour (baseColour);
        g.fillRoundedRectangle (bounds, cornerSize);

        // Borde fino
        g.setColour (Theme::accent.withAlpha(0.5f));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }

    // 2. DIBUJAR COMBOBOX (Menús desplegables)
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override
    {
        auto cornerSize = 4.0f;
        juce::Rectangle<int> boxBounds (0, 0, width, height);

        // Fondo oscuro
        g.setColour (Theme::background);
        g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

        // Borde
        g.setColour (box.findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

        // Flecha hacia abajo (triángulo)
        juce::Path path;
        path.startNewSubPath ((float) (buttonX + buttonW * 0.5f - 5.0f), (float) (buttonY + buttonH * 0.5f - 2.0f));
        path.lineTo ((float) (buttonX + buttonW * 0.5f + 5.0f), (float) (buttonY + buttonH * 0.5f - 2.0f));
        path.lineTo ((float) (buttonX + buttonW * 0.5f), (float) (buttonY + buttonH * 0.5f + 3.0f));
        path.closeSubPath();

        g.setColour (Theme::accent);
        g.fillPath (path);
    }
    
    // 3. DIBUJAR EL POPUP MENU (La lista que se abre al clicar el ComboBox)
    // Esto es importante, si no la lista seguirá siendo gris/blanca.
    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override
    {
        g.fillAll (Theme::background);
        g.setColour (Theme::gridLines);
        g.drawRect (0, 0, width, height);
    }
    
    // Dibujado de la ventana de runtime y configtab

        // 1. Definir el tamaño de la pestaña
        int getTabButtonBestWidth(juce::TabBarButton& button, int tabDepth) override
        {
            return 120; // Ancho fijo o dinámico, 120px se ve bien
        }

        // 2. Dibujar la pestaña (Tab)
        void drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
        {
            auto activeArea = button.getActiveArea();
            
            // Determinar colores
            auto tabBackground = button.isFrontTab() ? Theme::background : Theme::background.darker(0.2f);
            auto textColour    = button.isFrontTab() ? Theme::accent : Theme::text.withAlpha(0.6f);
            
            // Relleno base
            g.setColour(tabBackground);
            g.fillRect(activeArea);
            
            // Hover (si pasas el mouse por encima)
            if (isMouseOver && !button.isFrontTab()) {
                g.setColour(Theme::text.withAlpha(0.1f));
                g.fillRect(activeArea);
            }

            // Texto de la pestaña
            g.setColour(textColour);
            g.setFont(button.isFrontTab() ? juce::FontOptions(15.0f, juce::Font::bold) : juce::FontOptions(15.0f));
            g.drawText(button.getButtonText(), activeArea, juce::Justification::centred, false);
            
            // INDICADOR DE SELECCIÓN (Línea inferior o superior)
            if (button.isFrontTab()) {
                g.setColour(Theme::accent);
                // Dibujar una línea de 3px en la parte inferior de la pestaña
                g.fillRect(activeArea.removeFromBottom(3));
                
                // Opcional: Un brillo suave
                // g.setGradientFill(juce::ColourGradient(Theme::accent.withAlpha(0.2f), activeArea.getBottomX(), activeArea.getBottom(),
                //                                      juce::Colours::transparentBlack, activeArea.getBottomX(), activeArea.getY(), false));
                // g.fillRect(activeArea);
            }
        }
    
};
//fuentes y altavoces construcro principal
class SourceComponent : public juce::Component {
public:
    SourceComponent(int id) : LoudSpeakerID(id) {
        setSize(50, 50);
        angleLabel.setText("S" + juce::String(id), juce::dontSendNotification);
        angleLabel.setJustificationType(juce::Justification::bottom);
        angleLabel.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
        angleLabel.setSize(60, 20);
        addAndMakeVisible(angleLabel);
    }
    void updateAngleLabel(float angleDegrees) {
        angleLabel.setText("S" + juce::String(LoudSpeakerID) + ":" +
            juce::String(angleDegrees, 1) + "°",
            juce::dontSendNotification);
    }
    void mouseDrag(const juce::MouseEvent& event) override;

    void setCentrePosition(juce::Point<float> newCenter) {
        setBounds(newCenter.x - getWidth() / 2,  // X centrado
            newCenter.y - getHeight() / 2, // Y centrado
            getWidth(),
            getHeight());
    }
    // En SourceComponent
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat();
        
        // 1. Calcular opacidad basada en la ganancia (feedback visual)
        // Mapeamos ganancia 0.0-1.0 a opacidad visual.
        // Usamos sqrt para que los cambios bajos sean más visibles (gamma correction aprox)
        float visualIntensity = std::sqrt(juce::jlimit(0.0f, 1.0f, gain));

        // 2. Dibujar el "Glow" (Halo exterior)
        if (visualIntensity > 0.01f) {
            g.setColour(Theme::accent.withAlpha(0.4f * visualIntensity));
            float glowSize = bounds.getWidth() * (0.5f + 0.5f * visualIntensity);
            auto center = bounds.getCentre();
            g.fillEllipse(center.x - glowSize/2, center.y - glowSize/2, glowSize, glowSize);
        }

        // 3. Dibujar el núcleo del altavoz
        // Si tiene ganancia, se ilumina. Si no, es gris oscuro.
        juce::Colour coreColour = visualIntensity > 0.0f
            ? Theme::accent.interpolatedWith(juce::Colours::white, 0.3f).withAlpha(0.8f + 0.2f * visualIntensity)
            : Theme::speakerOff;

        g.setColour(coreColour);
        // Reducimos un poco el círculo central para dejar espacio al texto y halo
        g.fillEllipse(bounds.reduced(10.0f));

        // 4. Borde estético
        g.setColour(juce::Colours::black);
        g.drawEllipse(bounds.reduced(10.0f), 2.0f);

        // 5. Texto (ID)
        g.setColour(juce::Colours::lightgrey); // Texto oscuro sobre botón brillante
        g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
        g.drawText(juce::String(LoudSpeakerID), getLocalBounds(), juce::Justification::centred, false);
    }
    
    void setGain(float newGain) {
        gain = newGain;
    }
    int LoudSpeakerID;
    float gain = 0.0f;
    juce::Label angleLabel;
};
class VSourceComponent : public juce::Component {
public:
    VSourceComponent(int id) :VSourceID(id) {
        setSize(60, 60);
        // PALETA DE COLORES PREDEFINIDA - Más armónico
        static const std::vector<juce::Colour> colourPalette = {
            juce::Colours::mediumpurple,
            juce::Colours::hotpink,
            juce::Colours::lawngreen,
            juce::Colours::orange,
            juce::Colours::greenyellow,
            juce::Colours::cornflowerblue,
            juce::Colours::gold,
            juce::Colours::lightseagreen,
            juce::Colours::mediumvioletred,
            juce::Colours::darkred,
            juce::Colours::indianred
        };

        setColour = colourPalette[id % colourPalette.size()];
        // Configurar label
        angleLabel.setText("0", juce::dontSendNotification);
        angleLabel.setJustificationType(juce::Justification::bottom);
        angleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        angleLabel.setSize(60, 20);
        addAndMakeVisible(angleLabel);
    }
    void updateAngleLabel(float angleDegrees) {
        angleLabel.setText(juce::String(angleDegrees, 1) + "°",
            juce::dontSendNotification);
    }
    void mouseDrag(const juce::MouseEvent& event) override;
    void setCentrePosition(juce::Point<float> newCenter) {
        setBounds(newCenter.x - getWidth() / 2,  // X centrado
            newCenter.y - getHeight() / 2, // Y centrado
            getWidth(),
            getHeight());
    }
    // En VSourceComponent
    void paint(juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced(4.0f); // Margen para el trazo

        // 1. Relleno semitransparente del color asignado
        g.setColour(setColour.withAlpha(0.3f));
        g.fillEllipse(bounds);

        // 2. Borde grueso (Anillo de control)
        g.setColour(setColour);
        g.drawEllipse(bounds, 3.0f);


        // 3. Texto (ID) flotando arriba a la derecha o centro
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(17.0f));
        // Dibujamos el número pequeño encima de la mira
        g.drawText(juce::String(VSourceID), getLocalBounds().translated(0, 0), juce::Justification::centred, false);
    }
    
    int VSourceID; juce::Colour setColour;
    juce::Label angleLabel;
};
//constructor principal
class MainComponent : public juce::Component {
public:
    bool isMidiConfiured() const;
    MainComponent();
    ~MainComponent() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
    void VBAPImplementation(juce::Point<float> top, float radius, float angle, const juce::MouseEvent& event);
    void updateGainDisplay();
    void updateSpeakerPositions();
    void loadMidiConfiguration();
    void updateSourcesLayout();
    void applyMidiConfigToVirtualSources();
    void sendMidiMessage(int midiChannel, int deviceChannel, int value);
    // Obtener ganancias para una fuente virtual específica
    struct VirtualSourceData {
        int sourceID;
        std::vector<float>gains;
        int midiChannel = 1;      // Canal MIDI (por defecto 1)
        int controlNumber = 0;
    };
    //_______1_______ y ---4---//para mapear las ganancias
    std::vector<float> getVirtualSourceGains(int sourceID) {
        const juce::ScopedLock lock(gainslock);
        for (const auto& vsData : virtualSourcesGains) {
            if (vsData.sourceID == sourceID) {
                return vsData.gains;
            }
        }
        return {};
    }
    juce::Label gainLabel;
    class RuntimeTab;
    class ConfigTab;
    juce::StringArray availableMidiOutputs;
    //____________________________________configuración pára cargar la información del excel________________________________________
    std::vector<MixConfig>loadConfigFromCSV(const juce::File& file) {
        std::vector<MixConfig> configs;
        juce::StringArray lines;
        file.readLines(lines);
        // Saltar la primera línea
        for (int mixIdx = 1; mixIdx < lines.size(); ++mixIdx) {
            juce::StringArray tokens;
            tokens.addTokens(lines[mixIdx], ";", "");
            if (tokens.size() >= 32 && tokens[0].startsWith("Mix")) {
                MixConfig mixCfg;
                mixCfg.mixName = tokens[0];
                mixCfg.totalChannels = 32;
                mixCfg.totalMixes = 16;
                // Procesar los 32 canales
                for (int ch = 1; ch <= 32; ++ch) {
                    juce::String hexStr = tokens[ch].trim();

                    // Asegurar longitud par
                    if (hexStr.length() % 2 != 0) {
                        hexStr = "0" + hexStr;
                    }
                    mixCfg.channelSysex.add(hexStr);
                    // Convertir a LSB y MSB
                    uint16_t value = static_cast<uint16_t>(hexStr.getHexValue32());
                    mixCfg.channelValues.push_back(value); // Guardar valor entero
                    // Depuración: Imprimir LSB/MSB
                    uint8_t lsb, msb;
                    hexToLSBMSB(hexStr, lsb, msb);
                    //   DBG("Mix " + mixCfg.mixName + " | Canal " + juce::String(ch) + ": HEX=" + hexStr
                     //      + " → LSB=" + juce::String((int)lsb) + ", MSB=" + juce::String((int)msb));
                }
                configs.push_back(mixCfg);
                //  DBG("Mix Load: " << mixCfg.mixName << " with " << mixCfg.channelSysex.size() << " channels");
            }
        }
        return configs;
        
    }
    //---------------------------------------------------------------------------*quitar----------------------------*//
    void debugPrintMixConfigs() {
        for (const auto& mix : MixConfigs) {
            DBG("Mix: " << mix.mixName);
            for (int ch = 0; ch < mix.channelSysex.size(); ++ch) {
                DBG("  Channel " << ch + 1 << ": " << mix.channelSysex[ch]);
            }
        }
    }


    std::unique_ptr<juce::MidiOutput> midiOutput;
    std::vector<MixConfig> MixConfigs;
private:
    ModernLookAndFeel modernLook;
    juce::TabbedComponent tabbedComponent{ juce::TabbedButtonBar::TabsAtTop };
    void updateMidiPorts();
    void buttonClicked(juce::Button* button);
    void updateVirtualSourceGain(int vsourceID, int speakerID, float gain);
    std::vector<VirtualSourceData> virtualSourcesGains;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::StringArray sysexData;
    int totalMixes = 0;
    int totalChannels = 0;
    juce::CriticalSection gainslock;
    juce::File getSourceDirectoryFile() const {
        return juce::File(__FILE__).getParentDirectory().getChildFile("BookLS9.csv");
    }
    juce::String getSysexHex(int channel, int mix) const {
        if (channel >= 0 && channel < totalChannels &&
            mix >= 0 && mix < totalMixes) {
            int index = (mix * totalChannels) + channel;
            return sysexData[index];
        }
        return "";
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
//Clase para la pestaña de runtime
class MainComponent::RuntimeTab : public juce::Component, public juce::Button::Listener {
public:
    MainComponent& getOwner() const { return owner; }
    RuntimeTab(MainComponent& parent) : owner(parent) {
        // Crear fuentes iniciales
        for (int i = 0; i < 2; ++i) {
            auto* source = new SourceComponent(i + 1);
            sources.add(source);
            addAndMakeVisible(source);
        }

        resetButton.onClick = [this] { buttonClicked(&resetButton); };
		
    }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button)override;
    juce::OwnedArray<SourceComponent> sources;
    juce::OwnedArray<VSourceComponent> Vsources;
    juce::TextButton resetButton{ "Reset Mixes gains" };
private:
    MainComponent& owner;
};
//Clase para la pestaña de configuración
class MainComponent::ConfigTab : public juce::Component {
public:
    MainComponent& getOwner() const {

        return owner;
    }
    juce::OwnedArray<SourceComponent> configSources;


    ConfigTab(MainComponent& parent) : owner(parent) {
        addAndMakeVisible(loudspeakersInput);
        loudspeakersInput.setText("2");
        loudspeakersInput.onReturnKey = [this] { owner.updateSourcesLayout();
            // 
            };
        addAndMakeVisible(midiOutPort);
        addAndMakeVisible(MidiChannelbox);
        MidiChannelbox.addItemList({ "1", "2", "3", "4", "5", "6", "7", "8",
            "9", "10", "11", "12", "13", "14", "15", "16" }, 1);
        MidiChannelbox.setText("1");

        midiOutPort.onChange = [this] {
            auto midiOuts = juce::MidiOutput::getAvailableDevices();
            auto selectedId = midiOutPort.getSelectedId();
            auto device = midiOuts[selectedId - 1];
            owner.midiOutput = juce::MidiOutput::openDevice(device.identifier);

            };
        addAndMakeVisible(midiInPort);
        addAndMakeVisible(MidiConfigbox);
        MidiConfigbox.addItem(" ", 1);
        MidiConfigbox.addItem("Yamaha LS9 cofig", 2);
        MidiConfigbox.addItem("Load another config", 3);

        MidiConfigbox.onChange = [this] {
            if (MidiConfigbox.getSelectedId() == 3) {
                owner.loadMidiConfiguration();
            }
            if (MidiConfigbox.getSelectedId() == 2) {
                juce::String configFileName;

                // Mapea el ID de ComboBox al nombre del archivo
                int selectedId = MidiConfigbox.getSelectedId();
                if (selectedId == 2) {
                    configFileName = "BookLS9.csv";
                }
                else if (selectedId == 4) {
                    //revision de archivos para otras consolas
                    configFileName = "configuracion_otro_metodo.csv";
                }
                else {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Error", "Option Not valid.", " ");
                    return;
                }

                juce::File configFile;
                
#ifdef JUCE_DEBUG
                // En modo debug busca el archivo en la carpeta Source
                juce::File projectFolder = juce::File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory();
                configFile = projectFolder.getChildFile("Source").getChildFile(configFileName);
                configFile = juce::File(__FILE__).getParentDirectory().getChildFile(configFileName); //agregado para macbook
         
#else
                // En modo release busca el archivo junto al ejecutable
                juce::File executableFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
                juce::File executableFolder = executableFile.getParentDirectory();
                configFile = executableFolder.getChildFile(configFileName);
#endif

                if (configFile.existsAsFile()) {
                    owner.MixConfigs = owner.loadConfigFromCSV(configFile);
                    //quitar//
                    owner.debugPrintMixConfigs();
                    if (owner.MixConfigs.empty()) {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Error", "Mixes not loaded. Please, verify CSV file.", " ");
                    }
                    else {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Config loaded!", "Mixes: " + juce::String(owner.MixConfigs.size()), "Ok");
                        owner.applyMidiConfigToVirtualSources();
                    }
                }
                else {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                        "Set up error:",
                        "Configuration file '" + configFileName + "'has not founded.",
                        "OK");
                }
            }
            };


        addAndMakeVisible(VirtualSourceInput);
        VirtualSourceInput.setText("1");
        VirtualSourceInput.onReturnKey = [this]
            {owner.updateSourcesLayout();
        owner.applyMidiConfigToVirtualSources();
                    auto* runtimeTab = dynamic_cast<RuntimeTab*>(owner.tabbedComponent.getTabContentComponent(0));
                    for (int mixIndex = 0; mixIndex < owner.MixConfigs.size(); ++mixIndex) {
                        for (int speakerIndex = 0; speakerIndex < runtimeTab->sources.size(); ++speakerIndex) {

                            // Verificar que tenemos valores para este mix y altavoz
                            if (mixIndex < owner.MixConfigs.size() &&
                                speakerIndex < owner.MixConfigs[mixIndex].channelValues.size()) {

                                uint16_t value = owner.MixConfigs[mixIndex].channelValues[speakerIndex];
                                uint8_t lsb, msb;
                                hexToLSBMSB(juce::String::toHexString(value).paddedLeft('0', 4), lsb, msb);

                                // Enviar mensaje MIDI para este mix y altavoz específico
                                owner.sendMidiMessage(1, 98, lsb);
                                owner.sendMidiMessage(1, 99, msb);
                                owner.sendMidiMessage(1, 38, 0);  // LSB gain = 0
                                owner.sendMidiMessage(1, 6, 0);   // MSB gain = 0

                                DBG("Reset Mix " << mixIndex + 1 << " Speaker " << speakerIndex + 1);
                            }
                        }
                    }
            };
        updateMidiDevices();
        for (int i = 0; i < 2; ++i) {
            auto* source = new SourceComponent(i + 1);
            source->setInterceptsMouseClicks(true, false);
            configSources.add(source);
            addAndMakeVisible(source);
        }
    }



    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::lightgrey);
        // Dibujar crculo mpequeo (escala 0.7)
        auto center = getLocalBounds().getCentre().toFloat();
        float radius = getHeight() * 0.28f;
        g.setColour(juce::Colours::darkgrey);
        g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 2.0f);
        g.fillAll(juce::Colours::lightgrey);
        g.setColour(juce::Colours::black);
        g.drawText("Enter Number of Loudspeakers:", 10, 10, 200, 25, juce::Justification::left);
        g.drawText("Enter Virtual Source Input:", 10, 35, 200, 25, juce::Justification::left);
        g.drawText("Select MIDI Out Port:", 10, 70, 200, 25, juce::Justification::left);
        g.drawText("Select MIDI In Port:", 10, 105, 200, 25, juce::Justification::left);
        g.drawText("Select MIDI Config", 10, 145, 200, 25, juce::Justification::left);
        g.drawText("Select MIDI Channel:", 400, 10, 200, 25, juce::Justification::left);

        g.setColour(juce::Colours::aliceblue);
        g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 1.5f);
        //dibujar ejes
        g.setColour(juce::Colours::antiquewhite);
        g.drawLine(center.x - radius, center.y, center.x + radius, center.y, 1.5f);
        g.drawLine(center.x, center.y - radius, center.x, center.y + radius, 1.5f);
        //dibujar lebels de los ejes
        g.setColour(juce::Colours::white);
        g.drawText("90", center.x + radius + 5, center.y - 10, 30, 20, juce::Justification::left);
        g.drawText("0", center.x - 10, center.y - radius - 20, 30, 20, juce::Justification::centred);
        g.drawText("270", center.x - radius - 35, center.y - 10, 30, 20, juce::Justification::right);
        g.drawText("180", center.x - 10, center.y + radius + 5, 30, 20, juce::Justification::centred);
    }
    void resized() override {
        loudspeakersInput.setBounds(220, 10, 100, 25);
        loudspeakersInput.setBounds(220, 10, 100, 25);
        midiOutPort.setBounds(220, 70, 300, 25);
        midiInPort.setBounds(220, 105, 200, 25);
        VirtualSourceInput.setBounds(220, 35, 100, 25);
        MidiConfigbox.setBounds(220, 145, 150, 25);
        MidiChannelbox.setBounds(530, 10, 100, 25);

        //-----------------------------Planteamiento-----------------------------------------------------------------------------------
        auto center = getLocalBounds().getCentre().toFloat();
        auto radius = juce::jmin(getWidth(), getHeight()) * 0.28f;
        const float angle1 = juce::MathConstants<float>::twoPi * 5.0f / 8.0f;
        const float angle2 = juce::MathConstants<float>::twoPi * 7.0f / 8.0f;
        if (configSources.size() >= 1) configSources[0]->setTopLeftPosition(
            center.x + radius * std::cos(angle1) - 20,
            center.y + radius * std::sin(angle1) - 20
        );
        if (configSources.size() >= 2) configSources[1]->setTopLeftPosition(
            center.x + radius * std::cos(angle2) - 20,
            center.y + radius * std::sin(angle2) - 20
        );
        if (configSources.size() >= 3) {
            const float angleStep = juce::MathConstants<float>::twoPi / configSources.size();
            for (int i = 0; i < configSources.size(); ++i) {
                float angle = angle2 + angleStep * (i - 1);
                configSources[i]->setTopLeftPosition(
                    center.x + radius * std::cos(angle) - 20,
                    center.y + radius * std::sin(angle) - 20
                );
            }
        }
        // Posicionar altavoces desde el config tab
        for (int i = 0; i < configSources.size(); ++i) {
            float angle = juce::MathConstants<float>::twoPi * i / configSources.size();
            configSources[i]->setCentrePosition({
                center.x + radius * std::cos(angle),
                center.y + radius * std::sin(angle)
                });
        }
        // Sincronizar con RuntimeTab al inicializar
        owner.updateSpeakerPositions();
    }
    //funcion de actualización dispositivos MIDI
    void updateMidiDevices() {

        midiOutPort.clear();
        auto midiOuts = juce::MidiOutput::getAvailableDevices();
        for (const auto& device : midiOuts)
            midiOutPort.addItem(device.name, midiOutPort.getNumItems() + 1);
        midiInPort.clear();
        auto midiIns = juce::MidiInput::getAvailableDevices();
        for (const auto& device : midiIns)
            midiInPort.addItem(device.name, midiInPort.getNumItems() + 1);

    }
    juce::TextEditor loudspeakersInput;
    juce::ComboBox midiOutPort;
    juce::ComboBox midiInPort;
    juce::TextEditor VirtualSourceInput;
    juce::ComboBox MidiConfigbox;
    juce::ComboBox MidiChannelbox;
private:
    MainComponent& owner;
};

