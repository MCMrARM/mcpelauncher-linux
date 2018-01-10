#pragma once

class CommandOrigin;
class CommandOutput;
namespace Automation { class AutomationClient; }

class CommandOutputSender {

public:

    static std::vector<mcpe::string> translate(std::vector<mcpe::string> const& v);

    /// @symbol _ZN19CommandOutputSenderC2ERN10Automation16AutomationClientE
    CommandOutputSender(Automation::AutomationClient& automationClient);

    virtual ~CommandOutputSender();

    virtual void send(CommandOrigin const& origin, CommandOutput const& output);

    /// @symbol _ZN19CommandOutputSender22registerOutputCallbackERKSt8functionIFvR19AutomationCmdOutputEE
    virtual void registerOutputCallback();

};