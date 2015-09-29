namespace QP {

/*! @page concepts Concepts and Structure

@tableofcontents

@section oop Object-Orientation

As most C++ frameworks, QP/C++ uses classes, inheritance, and polymorphism as the main mechanisms for customizing the framework into applications. The framework is also layered and consists of components with well defined responsibilities.


------------------------------------------------------------------------------
@section ao Active Objects

<strong>Active Objects</strong> (a.k.a. actors) are encapsulated @ref sm "state machines" that run in their own thread of execution and process events asynchronously using an event-driven receive loop. They inherently support and automatically enforce the best practices of concurrent programming, such as: keeping the thread's data local and bound to the thread itself, asynchronous inter-thread communication without blocking, and using state machines instead of convoluted `IF-ELSE` logic (a.k.a. "spaghetti" code). In contrast, raw RTOS-based threading lets you do anything and offers no help or automation for the best practices.

@htmlonly
<div class="image">
<a target="_blank" href="http://www.state-machine.com/doc/AN_Active_Objects_for_Embedded.pdf"><img border="0" src="img/AN_Active_Objects_for_Embedded.jpg" title="Download PDF"></a>
<div class="caption">
Application Note: Active Objects for Embedded Systems
</div>
</div>
@endhtmlonly

The Quantum Leaps Application Note <a class="extern" target="_blank" href="http://www.state-machine.com/doc/AN_Active_Objects_for_Embedded.pdf"><strong>Active Objects for Embedded Systems</strong></a> describes the Active Object design pattern in the context of real-time embedded systems.
<div class="clear"></div>


------------------------------------------------------------------------------
@section sm State Machines

The behavior of each active object in QP/C++ is specified by means of a **hierarchical state machine** (UML statechart), which is the most effective and elegant technique of decomposing event-driven behavior. The most important innovation of UML state machines over classical finite state machines (FSMs) is the hierarchical state nesting. The value of state nesting lies in avoiding repetitions, which are inevitable in the traditional "flat" FSM formalism and are the main reason for the "state-transition explosion" in FSMs. The semantics of state nesting allow substates to define only the differences of behavior from the superstates, thus promoting sharing and reusing behavior.

@htmlonly
<div class="image">
<a target="_blank" href="http://www.state-machine.com/doc/AN_Crash_Course_in_UML_State_Machines.pdf"><img border="0" src="img/AN_Crash_Course_in_UML_State_Machines.jpg" title="Download PDF"></a>
<div class="caption">
Application Note: A Crash Course in UML State Machines
</div>
</div>
@endhtmlonly

The Quantum Leaps Application Note <a class="extern" target="_blank" href="http://www.state-machine.com/doc/AN_Crash_Course_in_UML_State_Machines.pdf"><strong>A Crash Course in UML State Machines</strong></a> introduces the main state machine concepts backed up by examples.
<div class="clear"></div>

@note
The hallmark of the QP/C++ implementation of UML state machines is **traceability**, which is direct, precise, and unambiguous mapping of every state machine element to human-readable, portable, MISRA-compliant C++ code. Preserving the traceability from requirements through design to code is essential for mission-critical systems, such as medical devices or avionic systems.


------------------------------------------------------------------------------
@section comp Components of QP/C++
The QP/C++ active object framework is comprised of the following components:

@image html qp_components.jpg "Components of the QP Framework"

<div class="separate"></div>
@subsection comp_qep QEP Hierarchical Event Processor
QEP is a universal, UML-compliant event processor that enables developers to code UML state machines in highly readable ANSI-C, in which every state machine element is mapped to code precisely, unambiguously, and exactly once (traceability). QEP fully supports hierarchical state nesting, which is the fundamental mechanism for reusing behavior across many states instead of repeating the same actions and transitions over and over again. (See also @ref qep).

<div class="separate"></div>
@subsection comp_qf QF Active-Object Framework
QF is a portable, event-driven, active-object (actor) framework for execution of **active objects** (concurrent state machines) specifically designed for real-time embedded (RTE) systems. (See also @ref qf).

<div class="separate"></div>
@subsection comp_qk QK Preemptive Kernel
QK is a tiny **preemptive**, priority-based, non-blocking, real-time kernel designed specifically for executing active objects. QK meets all the requirement of the <a class="extern" target="_blank" href="http://en.wikipedia.org/wiki/Rate-monotonic_scheduling"><strong>Rate Monotonic Scheduling</strong></a> (a.k.a. Rate Monotonic Analysis &mdash; RMA) and can be used in hard real-time systems. (See also @ref qk). 

<div class="separate"></div>
@subsection comp_qv  QV Cooperative Kernel
QV is a simple **cooperative** kernel (previously called "Vanilla" kernel). This kernel executes active objects one at a time, with priority-based scheduling performed before processing of each event. Due to naturally short duration of event processing in state machines, the simple QV kernel is often adequate for many real-time systems. (See also @ref qv). 

<div class="separate"></div>
@subsection comp_qs QS Software Tracing System
QS is software tracing system that enables developers to monitor live event-driven QP applications with minimal target system resources and without stopping or significantly slowing down the code. QS is an ideal tool for testing, troubleshooting, and optimizing QP applications. QS can even be used to support acceptance testing in product manufacturing. (See also @ref qs).


------------------------------------------------------------------------------
@section classes Classes in QP/C++
The figure below shows the main classes comprising the QP/C++ framework and their relation to the application-level code, such as the @ref game example application.

@image html qp_classes.png "Main Classes in the QP Framework"

<ul class="tag">
  <li><span class="tag">0</span> The QEvt class represents events without parameters and serves as the base class for derivation of time events and any events with parameters. For example, application-level events `ObjectPosEvt` and `ObjectImageEvt` inherit QEvt and add to it some parameters (see <span class="tag">8</span>). 
  </li>
  
  <li><span class="tag">1</span> The abstract QMsm class represents the most fundamental State Machine in QP/C++. This class implements the fastest and the most efficient strategy for coding hierarchical state machines, but this strategy is not human-maintainable and requires the use of the <a class="extern" target="_blank" href="http://www.state-machine.com/qm">QM modeling tool</a>. The class is abstract, meaning that it is not designed to be instantiated directly, but rather only for inheritance. The @ref game application provides an example of application-level classes deriving directly from QMsm (see <span class="tag">7</span>).
  </li>

  <li><span class="tag">2</span> The abstract QHsm class derives from QMsm and implements the state machine coding strategy suitable for manual coding and maintaining the code. The QHsm strategy is also supported by the <a class="extern" target="_blank" href="http://www.state-machine.com/qm">QM modeling tool</a>, but is not as fast or efficient as the QMsm strategy.
  </li>
  
  <li><span class="tag">3</span> The abstract QMActive class represents an active object that uses the QMsm style state machine implementation strategy. This strategy requires the use of the QM modeling tool to generate state machine code automatically, but the code is faster than in the QHsm style implementation strategy and needs less run-time support (smaller event-processor).
  </li>

  <li><span class="tag">4</span> The abstract QActive class represents an active object that uses the QHsm style implementation strategy for state machines. This strategy is tailored to manual coding, but it is also supported by the QM modeling tool. The resulting code is slower than in the QMsm-style implementation strategy.
  </li>

  <li><span class="tag">5</span> The QTimeEvt class represents time events in QP. **Time events** are special QP events equipped with the notion of time passage. The basic usage model of the time events is as follows. An active object allocates one or more QTimeEvt objects (provides the storage for them). When the active object needs to arrange for a timeout, it arms one of its time events to fire either just once (one-shot) or periodically. Each time event times out independently from the others, so a QP application can make multiple parallel timeout requests (from the same or different active objects). When QP detects that the appropriate moment has arrived, it inserts the time event directly into the recipient's event queue. The recipient then processes the time event just like any other event.
  </li>

  <li><span class="tag">6</span> Active Objects in the application derive either from the QMActive or QActive base class. 
  </li>

  <li><span class="tag">7</span> Applications can also use classes derived directly from the QMsm or QHsm base classes to represent "raw" state machines that are not active objects, because they don't have event queue and execution thread. Such "raw" state machines are typically used as "Orthogonal Components".
  </li>

  <li><span class="tag">8</span> Application-level events with parameters derive from the QEvt class.
  </li>
  
</ul>

@next{exa}
*/

} // namespace QP
