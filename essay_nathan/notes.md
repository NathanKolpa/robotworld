
# Arch

Het rijden van een robot wordt slecht gedaan met multithreading:

- bijna iedere operatie die wordt uitgevoerd is een race condition.
- Omdat c++ geen semantiek heeft voo thread-safety, moet dit in documentatie opgenomen worden dit wordt niet gedaan
  waardoor ik **alle** code moet lezen voordat ik weet hoe ik het kan gebruiken.
- moeilijk te testen.

Volgens wxwidgets:
> When writing a multi-threaded application, it is strongly recommended that no secondary threads call GUI functions.
> The design which uses one GUI thread and several worker threads which communicate with the main one using events is
> much
> more robust and will undoubtedly save you countless problems (example: under Win32 a thread can only access GDI
> objects
> such as pens, brushes, device contexts created by itself and not by the other threads).

1. een secudaire thread wordt in startDriving aangeroepen
2. deze thread roept notifyObservers aan.
3. deze notify gaat van robot naar robotworld naar robotworldcanvas, render en naar een wxWidgetCall

MainFrameWindow doet te veel:
tekent de gui.
Handeld command line agruments.
handeld gui events af.
is een super grote controller.

In plaats van arguments in de MainFrameWindow:

- Dependency inversion in de constructor voor server/client
- Ws een factory van server/client
  Commandline arguments bepalen de concrete instantie van deze objecten.
  Nu is het command line gedeelte verantwoordelijk voor arguments enzo.
  Nu is mainFrameWindow hoog level zoals een hoog level object hoort te zijn.

Server heeft veel dingen in de header staan in plaats van cpp
Gebruik asjeblieft early returns, vooral in server.hpp

Gebruik van comments:

- /* static */
- Zo veel weg gecommente code...
- Astar.cpp
- Lege doxygen comments:
- Logger.cpp
- Astar.cpp

# Patterns

Observer pattern:

- Notifier.cpp
- Observer.cpp

Strategy Pattern:

- ITraceFunction.hpp
- FileTraceFunction
- WidgetTraceFunction
- CoutTraceFunction

Singleton pattern:

- getCommunicationService
- robotworld
