#pragma once

class CommandOrigin;
class CommandOutput;

class CommandOutputSender {

public:

    static void (*CommandOutputSender_construct)(CommandOutputSender*, Automation::AutomationClient&);
    static void (*CommandOutputSender_destruct)(CommandOutputSender*);
    static void (*CommandOutputSender_send)(CommandOutputSender*, CommandOrigin const&, CommandOutput const&);
    static std::vector<mcpe::string> (*CommandOutputSender_translate)(std::vector<mcpe::string> const&);

    static std::vector<mcpe::string> translate(std::vector<mcpe::string> const& v) {
        return CommandOutputSender_translate(v);
    }

    CommandOutputSender(Automation::AutomationClient& automationClient) {
        CommandOutputSender_construct(this, automationClient);
    }

    virtual ~CommandOutputSender() {
        CommandOutputSender_destruct(this);
    }

    virtual void send(CommandOrigin const& origin, CommandOutput const& output) {
        CommandOutputSender_send(this, origin, output);
    }

    virtual void registerOutputCallback() {  }

};