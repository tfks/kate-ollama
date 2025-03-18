#ifndef OLLAMA_PLUGIN_H
#define OLLAMA_PLUGIN_H

#include <KTextEditor/Plugin>
#include <QObject>

class OllamaPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    explicit OllamaPlugin(QObject *parent = nullptr);
    ~OllamaPlugin() override;

private:
    void runOllama();  // La dichiarazione corretta della funzione privata

    // Dobbiamo implementare createView() perché è un metodo puro virtuale
    QObject *createView(KTextEditor::MainWindow *mainWindow) override;
};

#endif // OLLAMA_PLUGIN_H
