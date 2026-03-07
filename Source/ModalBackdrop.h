#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// =============================================================================
// ModalBackdrop - Reusable full-screen modal backdrop for picker panels
// =============================================================================
// Usage:
// 1. Create your picker panel component
// 2. Show modal: ModalBackdrop::show(pickerPanel, onDismiss, onSelect)
// 3. Clicking backdrop dismisses without selection
// 4. Clicking inside panel uses panel's onSelect callback
// =============================================================================

template<typename PickerPanelType>
class ModalBackdrop : public juce::Component
{
public:
    ModalBackdrop(PickerPanelType* panel, std::function<void()> dismissCallback)
        : pickerPanel(panel), onDismiss(dismissCallback)
    {
        setInterceptsMouseClicks(true, true);
        addAndMakeVisible(pickerPanel);
    }

    ~ModalBackdrop() override
    {
        // Panel is owned by unique_ptr in show(), will be deleted automatically
    }

    void paint(juce::Graphics& g) override
    {
        // Dark semi-transparent backdrop
        g.setColour(juce::Colour(0xcc000000));
        g.fillRect(getLocalBounds());
        
        // Hint text at bottom
        g.setColour(juce::Colour(0x88ffffff));
        g.setFont(juce::Font("Courier New", 10.0f, juce::Font::plain));
        g.drawText("tap outside to cancel",
                   getLocalBounds().withTrimmedBottom(6),
                   juce::Justification::centredBottom);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        // Click outside panel = dismiss
        if (!pickerPanel->getBounds().contains(e.getPosition()))
        {
            if (onDismiss)
                onDismiss();
        }
    }

    // Swallow all other mouse events so nothing beneath receives them
    void mouseUp(const juce::MouseEvent&) override {}
    void mouseDrag(const juce::MouseEvent&) override {}
    void mouseMove(const juce::MouseEvent&) override {}

    // Static helper to show modal with automatic cleanup
    static void show(juce::Component* parent,
                     std::unique_ptr<PickerPanelType> panel,
                     std::function<void()> onDismiss,
                     int width, int height)
    {
        auto* modal = new ModalBackdrop<PickerPanelType>(panel.release(), onDismiss);
        modal->setBounds(parent->getLocalBounds());
        
        // Center the picker panel
        modal->pickerPanel->setBounds(
            (modal->getWidth() - width) / 2,
            (modal->getHeight() - height) / 2,
            width, height
        );
        
        parent->addAndMakeVisible(modal);
        modal->toFront(true);
        
        // Store in component so it can delete itself
        modal->selfReference.reset(modal);
    }

    void dismiss()
    {
        if (auto* parent = getParentComponent())
            parent->removeChildComponent(this);
        selfReference.reset();  // Deletes this
    }

    PickerPanelType* pickerPanel;  // Public access for positioning

private:
    std::function<void()> onDismiss;
    std::unique_ptr<ModalBackdrop> selfReference;
};
