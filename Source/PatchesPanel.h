#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BuiltInPatches.h"
#include "PatchSerializer.h"

// =============================================================================
// PatchesPanel - List of built-in patches with code preview
// =============================================================================
class PatchesPanel : public juce::Component, public juce::ListBoxModel
{
public:
    std::function<void()> onClose;
    std::function<void(int)> onPatchSelected;
    std::function<void(const YM2612Patch&, int, int, int)> onPatchLoaded;  // Load patch into synth
    
    PatchesPanel(const YM2612Patch& currentPatch, int currentBlock, int currentLfoEnable, int currentLfoFreq)
        : patchList("Patches", nullptr), 
          selectedPatch(0), 
          codeModified(false),
          originalPatch(currentPatch),
          originalBlock(currentBlock),
          originalLfoEnable(currentLfoEnable),
          originalLfoFreq(currentLfoFreq)
    {
        setInterceptsMouseClicks(true, true);
        
        // Code display (left side) - now editable
        codeDisplay.setMultiLine(true);
        codeDisplay.setReadOnly(false);  // Editable
        codeDisplay.setReturnKeyStartsNewLine(true);  // Enable enter key
        codeDisplay.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain));
        codeDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF0D0D1A));
        codeDisplay.setColour(juce::TextEditor::textColourId, juce::Colour(0xFF00D4AA));
        codeDisplay.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF252540));
        
        // Serialize first patch to display
        auto& firstEntry = kBuiltInPatches[0];
        codeDisplay.setText(PatchSerializer::serializePatch(*firstEntry.patch, firstEntry.name,
                                                            firstEntry.block, firstEntry.lfoEnable, firstEntry.lfoFreq));
        
        codeDisplay.onTextChange = [this]() { 
            codeModified = true; 
            validateButton.setEnabled(true);
        };
        addAndMakeVisible(codeDisplay);
        
        // Original patch display (read-only)
        originalPatchDisplay.setMultiLine(true);
        originalPatchDisplay.setReadOnly(true);
        originalPatchDisplay.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
        originalPatchDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF0D0D1A));
        originalPatchDisplay.setColour(juce::TextEditor::textColourId, juce::Colour(0xFF888888));
        originalPatchDisplay.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF252540));
        originalPatchDisplay.setText(PatchSerializer::serializePatch(originalPatch, "ORIGINAL_PATCH",
                                                                      originalBlock, originalLfoEnable, originalLfoFreq));
        addAndMakeVisible(originalPatchDisplay);
        
        // Original patch label
        originalPatchLabel.setText("Original Patch (when panel opened):", juce::dontSendNotification);
        originalPatchLabel.setFont(juce::Font("Courier New", 10.0f, juce::Font::bold));
        originalPatchLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF888888));
        addAndMakeVisible(originalPatchLabel);
        
        // Error display
        errorLabel.setFont(juce::Font("Courier New", 10.0f, juce::Font::plain));
        errorLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFFF4444));
        errorLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF1a1a2e));
        errorLabel.setJustificationType(juce::Justification::centredLeft);
        errorLabel.setText("", juce::dontSendNotification);
        addAndMakeVisible(errorLabel);
        
        // Validate & Load button
        validateButton.setButtonText("Validate & Load");
        validateButton.setEnabled(false);  // Disabled until code is edited
        validateButton.onClick = [this]() { validateAndLoadPatch(); };
        addAndMakeVisible(validateButton);
        
        // Copy Code button
        copyButton.setButtonText("Copy All Code");
        copyButton.onClick = [this]() { 
            juce::SystemClipboard::copyTextToClipboard(codeDisplay.getText());
            errorLabel.setText("✓ Code copied to clipboard", juce::dontSendNotification);
            errorLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00FF88));
        };
        addAndMakeVisible(copyButton);
        
        // OK button (accept current patch and close)
        okButton.setButtonText("OK");
        okButton.onClick = [this]() {
            if (onClose)
                onClose();
        };
        addAndMakeVisible(okButton);
        
        // Cancel button (restore original patch and close)
        cancelButton.setButtonText("Cancel");
        cancelButton.onClick = [this]() {
            // Restore original patch
            if (onPatchLoaded)
                onPatchLoaded(originalPatch, originalBlock, originalLfoEnable, originalLfoFreq);
            if (onClose)
                onClose();
        };
        addAndMakeVisible(cancelButton);
        
        // Patch list (right side)
        patchList.setModel(this);
        patchList.setRowHeight(28);
        patchList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xFF0D0D1A));
        patchList.setColour(juce::ListBox::outlineColourId, juce::Colour(0xFF252540));
        patchList.selectRow(0);
        addAndMakeVisible(patchList);
    }

    void paint(juce::Graphics& g) override
    {
        // Panel background
        g.setColour(juce::Colour(0xf51a1a2e));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.f);
        
        // Border
        g.setColour(juce::Colour(0xFF00D4AA));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f), 10.f, 2.f);
        
        // Title
        g.setColour(juce::Colour(0xFF00D4AA));
        g.setFont(juce::Font("Courier New", 14.f, juce::Font::bold));
        g.drawText("Built-in Patches", getLocalBounds().withHeight(40).reduced(16, 0),
                   juce::Justification::centredLeft);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(16);
        bounds.removeFromTop(40); // Title area
        bounds.removeFromTop(8);  // Spacing
        
        // Button row at bottom: OK, Cancel, Copy
        auto buttonRow = bounds.removeFromBottom(36);
        cancelButton.setBounds(buttonRow.removeFromLeft(100));
        buttonRow.removeFromLeft(8);
        okButton.setBounds(buttonRow.removeFromLeft(100));
        buttonRow.removeFromLeft(16);
        copyButton.setBounds(buttonRow.removeFromLeft(120));
        
        bounds.removeFromBottom(8); // Space above buttons
        
        // Validate button and error label
        auto validateArea = bounds.removeFromBottom(30);
        validateButton.setBounds(validateArea.removeFromLeft(140));
        validateArea.removeFromLeft(8);
        errorLabel.setBounds(validateArea);
        
        bounds.removeFromBottom(8); // Space above validate section
        
        // Original patch section at bottom
        auto originalArea = bounds.removeFromBottom(120);
        originalPatchLabel.setBounds(originalArea.removeFromTop(20));
        originalPatchDisplay.setBounds(originalArea);
        
        bounds.removeFromBottom(8); // Space above original section
        
        // Split remaining area: code on left (60%), list on right (40%)
        auto listArea = bounds.removeFromRight(bounds.getWidth() * 0.4f);
        listArea.removeFromLeft(8); // Gap between code and list
        
        codeDisplay.setBounds(bounds);
        patchList.setBounds(listArea);
    }
    
    // ListBoxModel methods
    int getNumRows() override
    {
        return kNumBuiltInPatches;
    }
    
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber < 0 || rowNumber >= getNumRows())
            return;
            
        auto& patch = kBuiltInPatches[rowNumber];
        
        // Background
        if (rowIsSelected)
            g.fillAll(juce::Colour(0xFF252550));
        else if (rowNumber % 2 == 0)
            g.fillAll(juce::Colour(0xFF161625));
        else
            g.fillAll(juce::Colour(0xFF0D0D1A));
        
        // Patch name (centered)
        g.setColour(rowIsSelected ? juce::Colour(0xFF00D4AA) : juce::Colour(0xFFCCCCCC));
        g.setFont(juce::Font("Courier New", 12.f, juce::Font::plain));
        g.drawText(patch.name, 8, 0, width - 16, height, juce::Justification::centredLeft);
    }
    
    void listBoxItemClicked(int row, const juce::MouseEvent&) override
    {
        if (row >= 0 && row < getNumRows())
        {
            selectedPatch = row;
            auto& entry = kBuiltInPatches[row];
            codeDisplay.setText(PatchSerializer::serializePatch(*entry.patch, entry.name,
                                                                entry.block, entry.lfoEnable, entry.lfoFreq));
            codeModified = false;
            validateButton.setEnabled(false);
            errorLabel.setText("", juce::dontSendNotification);
            
            // Load patch into synth immediately
            if (onPatchLoaded)
                onPatchLoaded(*entry.patch, entry.block, entry.lfoEnable, entry.lfoFreq);
        }
        
        if (onPatchSelected)
            onPatchSelected(row);
    }
    
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override
    {
        if (onPatchSelected)
            onPatchSelected(row);
        if (onClose)
            onClose();
    }
    
    void validateAndLoadPatch()
    {
        juce::String code = codeDisplay.getText();
        juce::String error;
        int errorLine = 0, errorCol = 0;
        YM2612Patch parsedPatch;
        int parsedBlock, parsedLfoEnable, parsedLfoFreq;
        
        if (PatchSerializer::parsePatch(code, parsedPatch, parsedBlock, parsedLfoEnable, parsedLfoFreq,
                                       error, errorLine, errorCol))
        {
            // Success - patch parsed correctly
            errorLabel.setText("✓ Patch valid and loaded", juce::dontSendNotification);
            errorLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00FF88));
            codeModified = false;
            validateButton.setEnabled(false);
            
            // Load the parsed patch into the synth immediately
            if (onPatchLoaded)
                onPatchLoaded(parsedPatch, parsedBlock, parsedLfoEnable, parsedLfoFreq);
            
            DBG("Parsed patch - ALG:" << parsedPatch.ALG << " FB:" << parsedPatch.FB 
                << " BLOCK:" << parsedBlock << " LFO:" << parsedLfoEnable << "/" << parsedLfoFreq);
        }
        else
        {
            // Error
            errorLabel.setText(juce::String::formatted("Invalid patch syntax at %d:%d - %s", 
                                                        errorLine, errorCol, error.toRawUTF8()), 
                               juce::dontSendNotification);
            errorLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFFF4444));
        }
    }

private:
    juce::ListBox patchList;
    juce::TextEditor codeDisplay;
    juce::TextEditor originalPatchDisplay;
    juce::Label originalPatchLabel;
    juce::TextButton validateButton;
    juce::TextButton copyButton;
    juce::TextButton okButton;
    juce::TextButton cancelButton;
    juce::Label errorLabel;
    int selectedPatch;
    bool codeModified;
    
    // Original patch state (when panel was opened)
    YM2612Patch originalPatch;
    int originalBlock;
    int originalLfoEnable;
    int originalLfoFreq;
};

// =============================================================================
// PatchesModal - Modal wrapper for patches panel
// =============================================================================
class PatchesModal : public juce::Component
{
public:
    PatchesPanel* panel;
    std::function<void()> onDismiss;
    
    PatchesModal(PatchesPanel* patchesPanel, std::function<void()> dismissCallback)
        : panel(patchesPanel), onDismiss(dismissCallback)
    {
        setInterceptsMouseClicks(true, true);
        addAndMakeVisible(panel);
        
        panel->onClose = [this]() {
            if (onDismiss)
                onDismiss();
        };
    }

    void paint(juce::Graphics& g) override
    {
        // Dark semi-transparent backdrop
        g.setColour(juce::Colour(0xcc000000));
        g.fillRect(getLocalBounds());
    }

    // Swallow all mouse events - no dismiss on backdrop click
    void mouseDown(const juce::MouseEvent&) override {}
    void mouseUp(const juce::MouseEvent&) override {}
    void mouseDrag(const juce::MouseEvent&) override {}
    void mouseMove(const juce::MouseEvent&) override {}

    void dismiss()
    {
        if (auto* parent = getParentComponent())
            parent->removeChildComponent(this);
        selfReference.reset();  // Deletes this
    }

    std::unique_ptr<PatchesModal> selfReference;
};
