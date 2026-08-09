#pragma once

class Pin
{
public:
    Pin();

    void setInput(bool state);
    bool getInput() const;

    void setOutput(bool state);
    bool getOutput() const;

private:
    bool inputState;
    bool outputState;
};