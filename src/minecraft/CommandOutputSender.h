#pragma once

class CommandOrigin;
class CommandOutput;
namespace Automation { class AutomationClient; }

class CommandOutputSender {

public:

    static void (*CommandOutputSender_destruct)(CommandOutputSender*);

    static std::vector<mcpe::string> translate(std::vector<mcpe::string> const& v);

    CommandOutputSender(Automation::AutomationClient& automationClient);

    virtual ~CommandOutputSender() {
        CommandOutputSender_destruct(this);
    }

    virtual void send(CommandOrigin const& origin, CommandOutput const& output);

    virtual void registerOutputCallback();

};