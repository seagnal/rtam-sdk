/***********************************************************************
 ** event.hh
 ***********************************************************************
 ** Copyright (c) SEAGNAL SAS
 **
 ** This software is the property of SEAGNAL and is protected
 ** by International laws on author rights, by the conventions and
 ** international treaties on author rights and any other applicable
 ** law.
 **
 ** User is not allowed to use, copy, modify, distribute, and sell
 ** this software and its documentation for any purpose.
 **
 ***********************************************************************/

/* define against mutual inclusion */
#ifndef RTAM_EVENT_HH_
#define RTAM_EVENT_HH_

/**
 * @file api.hh
 * This plugin allows the advanced control of SEAGNAL's RTAM modules.
 *In particular, it allows the configuration of emissions, the configuration of the digitization and processing chain, the configuration of recurrences over several scenarios, the configuration of synchronizations between modules, the feedback of digitized data as well as the feedback of the status of the different modules
 *
 * @author SEAGNAL (johann.baudy@seagnal.fr)
 * @date 2013
 *
 * @version 1.0 Original version
 */

/***********************************************************************
 * Includes
 ***********************************************************************/
#include <c/common.h>
#include <event_base.hh>

#ifndef ID_GENERATION
#include <rtam/api.autogen.hh>
#endif
/***********************************************************************
 * Namespace
 ***********************************************************************/
namespace master {
namespace plugins {
namespace fec {

/***********************************************************************
 * Defines
 ***********************************************************************/
#ifdef ID_GENERATION
enum ET_ID_BASE_RTAM {
  // Hw sub subnodes
  /** Hardware identifier.
  * Description: hardware module identifier.
  * Data: uint32_t
  * Parent: E_ID_PING_INFO, E_ID_TIME_INFO
  */
  E_ID_HW_UID,
#if 0
  /** Hardware identifier.
  * Description: hardware module identifier.
  * Data: uint64_t
  * Parent: E_ID_INIT_HW, E_ID_IIC, E_ID_HW_VERSION, E_ID_SPI_ADC
  */
  E_ID_HW_UID__IGNORE__,
#endif
  /** Hardware generale purpose input.
  * Description: hardware purpose input content
  * Data: uint32_t
  * Parent: E_ID_IIC
  */
  E_ID_HW_GPI,

  /** Hardware description.
  * Description: This node contains the description of a module.
  * This node is used in the module feature buffer in order to describe the module
  * Data: char[]
  * Port: E_PORT_INTERNAL_FEATURE (IN)
  */
  E_ID_HW_DESCRIPTION,

  /** Hardware name description.
  * Description: This node contains the name of the module.
  * This node is used in module feature buffer as standalone node in order to describe the module
  * Data: char[]
  * Parent: E_ID_HW_DESCRIPTION
  */
  E_ID_HW_NAME,

  /** Hardware type.
  * Description: This node indicates the nature of the entity
  * Data: uint8_t (Enum)
  *    - E_SYST_TYPE_SYSTEM (Trigger system)
  *    - E_SYST_TYPE_ADC (Analog to Digital Control interface)
  *    - E_SYST_TYPE_ADC_SPI (Analog to Digital Spi interface)
  *    - E_SYST_TYPE_PULSER (PWM Generator),
  *    - E_SYST_TYPE_IIC (IIC reader),
  *    - E_SYST_TYPE_DAC (Digital to Analog interface),
  * Childs: None
  * Parent: E_ID_INIT_HW, E_ID_HW_DESCRIPTION
  */
  E_ID_HW_TYPE,

  /** Hardware function identifier (OBSOLETE).
  * Parent: E_ID_INIT_HW
  */
  E_ID_HW_FUNCTION_ID,

  /** Internal identifier.
  * Description: internal numbering of the FPGA Rx32 entity.
  * If several entities of the same type are present on the same module, this node allows them to be identified
  * Data: uint32_t
  * Parent: E_ID_HW_TYPE, E_ID_INIT_HW
  */
  E_ID_HW_INTERNAL_ID,


  /** Hardware routing map.
  * Data: uint8_t[4]
  * All bytes are used independently by the internal switches of the Rx32 modules, using their id as index.
  * Description: Routing map of packet. This node is used to configure the access path to the entity.
  * Childs: None
  * Parent: E_ID_HW_TYPE
  */
  E_ID_HW_ROUTING,

  /** Hardware disabler.
  * Description: This node allows operation in a degraded mode. The corresponding entity is deactivated while maintaining the same output formats.
  * Data: uint8_t
  * - 0: Enabled (Default)
  * - 1: Disabled
  * Parent: E_ID_INIT_HW
  */
  E_ID_HW_DISABLED,

  /** Hardware channel Number.
  * Description: This node describes the number of channels contained by the entity
  * Data: uint32_t
  * Parent: E_ID_HW_TYPE, E_ID_INIT_HW
  */
  E_ID_HW_CHANNEL_NUMBER,

  /** Hardware architecture.
  * Description: This node gives an architecture type for an entity (ex: AD7768, AD9670)
  * Data: char []
  * Parent: E_ID_HW_TYPE
  */
  E_ID_HW_ARCHITECTURE,

  /** Hardware version.
  * Description: Hardware module version (dynamic size). This node is also used in module feature buffer as standalone node.
  * ex "7768" for AD7768 on E_SYST_TYPE_ADC
  * ex "9670" for AD9670 on E_SYST_TYPE_ADC
  * Data: char[]
  * Format used is [tag]-[commit-since-tag]-g[commit_uid]
  * Parent: E_ID_READY
  */
  E_ID_HW_VERSION,

#if 0
  /** Hardware version.
  * Description: Hardware module version (dynamic size). This node is also used in module feature buffer as standalone node.
  * Data: char[]
  * Format used is [tag]-[commit-since-tag]-g[commit_uid]
  * Parent: E_ID_HW_DESCRIPTION
  */
  E_ID_HW_VERSION__IGNORE__,
#endif
  /** Hardware time.
  * Description: Hardware module version (dynamic size). (Not used in E_ID_INIT_HW)
  * Time since the start-up. Measured in clock pulses.
  * Data: uint64_t
  * Time in ns from 1 jan 1970,
  * Parent: E_ID_HW_UID
  */
  E_ID_HW_TIME,


  // Commands
  /** Initiatization of Plugin.
  * Description: This node should contain hardware description if multi-system is needed.
  * Port: E_PORT_CLIENT (IN)
  * Data: None
  */
  E_ID_INIT,

  /** Scenarios configuration.
  * Description: This node describes the operation of the module in transmission and reception as well as the settings for generating triggers.
  * It will also trigger the transfer of configurations to the modules.
  * Port: E_PORT_CLIENT (IN)
  * Data: None
  */
  E_ID_CONFIG,

  /** Start request.
  * Description: This node starts the generation of triggers for the master-set modules.
  * Port: E_PORT_CLIENT (IN)
  * Data: None
  */
  E_ID_SYNC_START,

  /** Stop request.
  * Description: This node stops the generation of triggers for the master-set modules.
  * Port: E_PORT_CLIENT (IN)
  * Data: None
  */
  E_ID_SYNC_STOP,

  /** Asynchronous gpo set.
  * Description: This node updated the gpo on a specific device during IDLE phase.
  * Caution E_ID_HW_GPO is still applied during configuration. User must ensure compatibility of GPIO settings across systems.
  * If same device, is used over multiple systems, apply same configuration over systems.
  * Port: E_PORT_CLIENT (IN)
  * Data: None
  */
  E_ID_SET_GPO,

  /** Close request.
  * Description: This node deletes the configuration of the scenarios and the system.
  * In the case of using the JTAG line to switch on the modules, the arrival of this node will also switch off the power to the modules.
  * Port: E_PORT_CLIENT (IN)
  * Data: None
  */
  E_ID_CLOSE,

  /**  RTAM readiness.
  * Description: For each reception of the configuration descriptor, the RTAM driver transmits this node.
  * The data indicates whether the RTAM is ready to accept an E_ID_CONFIG.
  * This node also contains the HW and SW version information.
  * Port: E_PORT_CLIENT (OUT)
  * Data: uint8_t
  * - 0: Not ready
  * - 1: Ready
  */
  E_ID_READY,

  /** Ping.
  * Description: This node contains the sonar data and all the information associated with its decoding in the form of a BML sub-node.
  * Port: E_PORT_CLIENT (OUT)
  * Data: uint16_t[], uint32_t[], float[]
  *
  * The detailed format depends on the content of the child node E_ID_PING_INFO.
  * The mapping of the channels (CHx) of the modules to the internal ADC entities (numbered from 0 to 3, Kadc) is as follows:
  * ChX = Kadc*8 + Kch. with Kadc from 0 to 4 and Kch from 0 to 7.
  * Each internal ADC entity has 8 channels.
  * At the output of the FEC driver, the channels of a system are sorted by the UID of the module (end of mac address) in an ascending manner , then by the ascending internal ADC entity number (0 to 3), and then by the ascending order of ADC channel (0 to 7).
  * 
  * ex:
  * Output channel 0 :  Uid 0x10, Adc 0 channel 0,
  * Output channel 1 :  Uid 0x10, Adc 0 channel 1,
  * Output channel 2 :  Uid 0x10, Adc 1 channel 0, 
  * Output channel 3 :  Uid 0x10, Adc 1 channel 1, 
  * Output channel 4 :  Uid 0x2a, Adc 0 channel 0, 
  * Output channel 5 :  Uid 0x2a, Adc 0 channel 1, 
  * Output channel 6 :  Uid 0x2a, Adc 1 channel 0, 
  * Output channel 7 :  Uid 0x2a, Adc 1 channel 1, 
  * Output channel 8 :  Uid 0x40, Adc 0 channel 0, 
  * Output channel 9 :  Uid 0x40, Adc 0 channel 1, 
  * Output channel 10 : Uid 0x40, Adc 1 channel 0,
  * Output channel 11 : Uid 0x40, Adc 1 channel 1
  *
  * Data are stored in channel major. Data[ChX+NbChannel*SampleX]. 
  *
  * Type of Data (int16,int32,...) depends on E_ID_FORMAT. 
  * Real and Imaginary order depends on E_ID_COMPLEX.
  *
  * see f_rtam_handle_ping() in example for more information.
  */
  E_ID_PING,

  /** Ping stats.
  * Description: This node contains the sonar stats data and all the information associated with its decoding in the form of a BML sub-node.
  * Port: E_PORT_CLIENT (OUT)
  * Data: None
  */
  E_ID_PING_STATS,

  /** Ping SPECGRAM stats.
  * Description: This node contains the sonar specgram data.
  * Data is a 2D array of E_ID_PING_STATS_NFFT*E_ID_PING_STATS_NBWIN values.
  * Unit is volt
  * Data: float[]
  * Parent: E_ID_PING_STATS
  */

  E_ID_PING_STATS_SPECGRAM,
  /** Ping PSD stats.
  * Description: This node contains the sonar PSD data.
  * Data is a 1D array of E_ID_PING_STATS_NFFT values
  * Unit is volt / sqrt(Hz)
  * Data: float[]
  * Parent: E_ID_PING_STATS
  */
  E_ID_PING_STATS_PSD,

  /** Ping rms stats.
  * Description: This node contains the energy of each beam.
  * Data is a 1D array of E_ID_NB_BEAM values.
  * Unit is volt**2
  * Data: float[]
  * Parent: E_ID_PING_STATS
  */
  E_ID_PING_STATS_RMS,

  /** Number of bins in spectral analysis.
  * Description: This node contains the number of bins used to perform frequency analysis.
  * Data: uint32_t
  * Parent: E_ID_PING_STATS
  */
  E_ID_PING_STATS_NFFT,
  /** Number of windows in spectral analysis.
  * Description: This node contains the number of windows used to perform frequency analysis.
  * Data: uint32_t
  * Parent: E_ID_PING_STATS
  */
  E_ID_PING_STATS_NBWIN,



  /** Time info.
  * Description: This node contains the time info during idle state.
  * One packet is sent per device
  * Port: E_PORT_CLIENT (OUT)
  * Data: None
  */
  E_ID_TIME_INFO,


  /** SPI-ADC monitoring.
  * Description: This node contains the value digitized of each channel of the ADS8168.
  * Data: uint16_t[]
  * Port: E_PORT_CLIENT (OUT)
  * Consecutive channel of ads8168 value. 0,1,2,3,4,5,6,7,0,1..
  */
  E_ID_SPI_ADC,

  /** SPI-ADC monitoring.
  * Description: This node contains the offset of current packet
  * Data: uint32_t
  * Parent: E_ID_SPI_ADC
  */
  E_ID_SPI_ADC_OFFSET,

  /** Scenarios configuration acknowledge.
  * Description: This node indicates the end of the module configuration.
  * Modules are ready to receive an E_ID_SYNC_START
  * Port: E_PORT_CLIENT (OUT)
  * Data: None
  */
  E_ID_CONFIG_READY,

  /** Scenarios update stopped acknowledge.
  * Description: This node indicates the transient stop state of the modules for configuration updates. Following this node, the modules are ready to receive configuration updates.
  * Port: E_PORT_CLIENT (OUT)
  * Data: None
  */
  E_ID_CONFIG_STOPPED,

  /** Scenarios update acknowledge.
  * Description: This node indicates the end of the module configuration update.
  * The triggers are automatically activated after the update.
  * Port: E_PORT_CLIENT (OUT)
  * Data: None
  */
  E_ID_CONFIG_UPDATED,

  /** System version.
  * Description: Version of the plugin.
  * Port: E_PORT_CLIENT (IN)
  * Data: char[].
  * Format used is [tag]-[commit-since-tag]-g[commit_uid]
  * Parent: E_ID_READY
  */
  E_ID_VERSION,

  /** System configuration.
  * Description: For each system, a node of this type is required. It contains the list of entities containing the system.
  * Id of system is set with the position of this node within parent node.
  * Data: None
  * Parent: E_ID_INIT
  */
  E_ID_INIT_SYST,

  /** Entity configuration.
  * Description: For each entity, a node of this type is required.
  * It contains in its sub-nodes the description of the expected entity (type, internal no, module uid, number of channels)
  * Data: None
  * Parent: E_ID_INIT_SYST
  */
  E_ID_INIT_HW,

  /** Systems affinity.
  * Description: This node can be a child of all the configuration nodes in the scenario (not only E_ID_CONFIG).
  * It indicates by a bitfiled to which system the parent node is applied.
  * If not present the node is applied to all systems
  * Data: uint64 (BitField)
  * Bit 0 = System 0.
  * Bit 1 = System 1
  * ...
  * Parent: E_ID_CONFIG, E_ID_SET_GPO
  */
  E_ID_AFFINITY_SYSTEM,

  /** Scenario affinity.
  * Description: This node can be a child of all the configuration nodes in the scenario (not only E_ID_CONFIG).
  * It indicates by a bitfiled to which scenario the parent node is applied.
  * If not present the node is applied to all scenarios
  * Data: uint64 (BitField)
  * Bit 0 = Scenario 0.
  * Bit 1 = Scenario 1.
  * ...
  *
  * Can be applied on:
  * - E_ID_PULSE (and subnodes)
  * - E_ID_TRIGGER_DURATION
  * - E_ID_TRIGGER_SHIFT
  * - E_ID_NB_SAMPLE
  *
  * Parent: E_ID_CONFIG
  */
  E_ID_AFFINITY_SCENARIO,

  // Configuration subnodes

  /** Number of scenario.
  * Number of scenarios to allocate.
  * The maximum number of scenarios depends on the hardware. Usually 4.
  * If this node is not provided, this is given by the number of sub-nodes  E_ID_SCENARIO
  * Data: uint8_t
  * Parent: E_ID_CONFIG
  */
  E_ID_NB_SCENARIO,

  /** Scenario configuration.
  * Description: This node contains in its sub-nodes the description of the mode of operation in transmission, reception and trigger.
  * It will be necessary to instantiate this node by type of scenario desired.
  * The assignment of a configuration to a scenario can either be done by the total description of each scenario or by using the E_ID_AFFINITY_SCENARIO nodes.
  * This node can be used several times. Until one per scenario.
  * Data: None
  * Parent: E_ID_CONFIG
  */
  E_ID_SCENARIO,

  /** Selection of channels to configure.
  * Description: This node allows the selection of the channels to be sampled.
  * The data defines the method of interpretation of the child nodes.
  * This selection is either made by white list (only listed channels are brought up) or by black list (only listed channels are disabled)
  * Used by ADC entities
  * Data: uint8_t (Enum)
  * - 0: Mode Black List(default)
  * - 1: Mode White List.
  * Parent: E_ID_SCENARIO
  */
  E_ID_CHANNELS,

  /** Number channel to change.
  * Description: Number of the channel to add or delete according to E_ID_CHANNELS data (while list or black list).
  * Used by ADC entities
  * Data: uint16_t (Nombre)
  * Parent: E_ID_CHANNELS
  */
  E_ID_CHANNEL,

  /** Multiplexer.
  * Description: This node allows you to select the type of channels you wish to sample.
  * Used by ADC entities
  * Data: uint8_t (Enum)
  * - E_ADC_MUX_INPUT = 0, First stage, ADC output without demod
  * - E_ADC_MUX_DEMOD, ADC output demodulated
  * - E_MUX_FIR, FIR FPGA output demodulated
  * - E_MUX_DECIM, Decimated FIR FPGA output demodulated
  * Parent: E_ID_SCENARIO
  */
  E_ID_MUX,

  /** Sampling frequency.
  * Description: Sampling frequency of ADCs (Hertz).
  * Used by ADC entities
  * Data: double
  * Parent: E_ID_SCENARIO
  */
  E_ID_FREQ_SAMPLING,

  /** Frequency of demodulation.
  * Description:  Frequency of demodulation of ADCs (Hertz).
  * Used by ADC entities
  * Data: double
  * Parent: E_ID_SCENARIO
  */
  E_ID_FREQ_DEMOD,

  /** Cutoff frequency.
  * Description: Cutoff frequency of FIR filtering (Hertz).
  * Used by ADC entities
  * Data: double
  * Parent: E_ID_SCENARIO
  */
  E_ID_FREQ_CUTOFF,

  /** Decimation.
  * Description: Total value of decimation (FPGA+ADC).
  * Sampling rate of the output stream will be the content of E_ID_FREQ_SAMPLING divided by the content of  E_ID_DECIMATE
  * Used by ADC entities
  * Data: uint16_t
  * Parent: E_ID_SCENARIO
  */
  E_ID_DECIMATE,

  /** Number of samples.
  * Description: This node contains the number of samples expected by the plugin for each trigger and each channel.
  * This value allows the allocation of the expected buffer and the control of missing data.
  * It must be set in accordance with the recurrence.
  * Used by ADC entities
  * This node is taken into account during the update procedure.
  * Data: uint32_t
  * Parent: E_ID_SCENARIO
  */
  E_ID_NB_SAMPLE,

  /** Test mode.
  * Description:  Selection of the plugin/modules test mode.
  * Used by ADC entities
  * Data: uint8_t
  * - E_ADC_TEST_MODE_DISABLE = 0, No Test mode (Default)
  * - E_ADC_TEST_MODE_SINE, (AD9670 only : Digital SINUS value*fsample div 64, fullscale )
  * - E_ADC_TEST_MODE_DECIMATOR_FILTER (AD9670: Integrated decimation coef),
  * - E_ADC_TEST_MODE_CHANNEL_ID (AD9670: Channel Id of each ADC),
  * - E_ADC_TEST_MODE_ANALOG_TONE (AD9670: Analog Tone Fsample div 32),
  * - E_ADC_TEST_MODE_USER_INPUT (AD9670: User custom input),
  * - E_ADC_TEST_MODE_DECIMATOR_FILTER_FPGA (FPGA: Dirac response of DECIM Filter),
  * - E_ADC_TEST_MODE_DECIMATOR_NO_ACCUMULATOR_GAIN (FPGA: No accumulator Gain on decim filter, low now analysis),
  * - E_ADC_TEST_MODE_ADC_ID, (Fec: Channel Id of each ADC),
  * - E_ADC_TEST_MODE_PRN_SHORT (AD9670: PRN output short sequence),
  * - E_ADC_TEST_MODE_PRN_LONG (AD9670: PRN output long sequence)
  * Parent: E_ID_SCENARIO
  */
  E_ID_TEST_MODE,


  /** Full scale.
  * Description: This node contains the full scale of the ADC converters in Volt.
  * The specified and returned values can be different.
  * Used by ADC entities
  * Data: double
  * Parent: E_ID_SCENARIO, E_ID_PING_INFO
  */
  E_ID_FULL_SCALE,

  /** Additional digital gain
  * Description: This node contains the digital gain of FIR.
  * Applied gain will be 2**value.
  * Used by ADC entities
  * Data: uint16
  * Parent: E_ID_SCENARIO
  */
  E_ID_DIGITAL_GAIN,

  /** Trigger mode.
  * Description: This node defines whether the trigger is internal or external
  * Used by system trigger entities
  * Data: uint8_t (Enum)
  * - E_TRIGGER_EXTERNAL = 0,
  * - E_TRIGGER_ACQUISITION_START (Internal trigger)
  * Parent: E_ID_SCENARIO
  */
  E_ID_TRIGGER_MODE,

  /** Pow sync frequency.
  * Description: This node defines the frequency of the POW_SYNC output corresponding to the array index.
  * The POW_SYNC outputs are PWM outputs with adjustable frequency and duty cycle.
  * Used by Trigger entities
  * Data: float[] (Hz)
  * Parent: E_ID_SCENARIO
  */
  E_ID_TRIGGER_POW_SYNC_FREQ,

  /** Pow sync duty cycle.
  * Description: This node defines the duty cycle of the POW_SYNC output corresponding to the array index.
  * The POW_SYNC outputs are PWM outputs with adjustable frequency and duty cycle.
  * Used by system trigger entities
  * Data: float[]
  * 0 = 0%
  * 1.0 = 100%
  * Parent: E_ID_SCENARIO
  */
  E_ID_TRIGGER_POW_SYNC_DUTY,

  /** Trigger duration.
  * Description: This node defines the interval between trigger of this scenario.
  * Used by system trigger entities
  * This node is taken into account during the update procedure.
  * Data: double (second)
  * Parent: E_ID_SCENARIO
  */
  E_ID_TRIGGER_DURATION,

  /** Trigger max counter.
  * Description: This node defines the maximum number of triggers before the state machines are stopped. 0 means infinite
  * Used by system trigger entities
  * Data: uint32_t
  * Parent: E_ID_SCENARIO
  */
  E_ID_TRIGGER_MAX,

  /** Master trigger Unique ID
  * Description: In case the trigger generation mode is E_TRIGGER_ACQUISITION_START. This node indicates the unique number of the card generating the trigger
  * Used by system trigger entities
  * Data: uint64_t
  * Parent: E_ID_SCENARIO
  */
  E_ID_TRIGGER_MASTER,


  /** Trigger latch mode
  * Description: The trigger signal can be encoded as AC (E_TRIGGER_LATCH_MODE_AC) or DC (E_TRIGGER_LATCH_MODE_NONE)
  * Used by system trigger entities
  * Data: uint8_t
  * - E_TRIGGER_LATCH_MODE_NONE = 0, DC mode
  * - E_TRIGGER_LATCH_MODE_AC, AC mode, 1:(12.5MHz) or 0 (6.25MHz)
  * Parent: E_ID_SCENARIO
  */
  E_ID_TRIGGER_LATCH_MODE,

  /** Trigger output shift
  * Description: The SYNC_TRIGGER signal can be offset by a certain amount of time from the internal trigger.
  * This node specifies it.
  * Used by system trigger entities
  * Data: double
  * Parent: E_ID_SCENARIO
  */
  E_ID_TRIGGER_SHIFT,


  /** PPS edge mode
  * Description: The PPS signal can be exoected on rising or falling edge
  * Used by system trigger entities
  * Data: uint8_t
  * - E_PPS_EDGE_MODE_FALLING = 0, Falling Edge
  * - E_PPS_EDGE_MODE_RISING, Rising Edge (Default)
  * Parent: E_ID_SCENARIO
  */
  E_ID_PPS_EDGE_MODE,


  /** Post trigger.
  * Description: This node defines the number of samples ignored after the internal trigger.
  * Used by adc entities. (Expermimental)
  * Data: uint32_t
  * Default: 0
  * Parent: E_ID_SCENARIO
  */
  E_ID_POST_TRIGGER,

  /** IIC Result
  * Description: This node contains the results of an IIC reading
  * Data: None
  * Port: E_PORT_CLIENT (OUT)
  */
  E_ID_IIC,

#if 0 //FIXME Update node
  /** IIC Config
  * Description: This node contains the IIC configuration.
  * The data contains the loopback offset of the IIC reading.
  * This is to segment the configuration into a configuration part that executes once, and the measurement part that executes every ping or every second depending on the operating mode
  * Data: uint32_t
  * Parent: E_ID_SCENARIO
  */
  E_ID_IIC__IGNORE__,
#endif
  /** IIC Read Option
  * Description: This node sets a read or write command. If present this is a read command.
  * Data: None
  * Parent: E_ID_IIC_CMD
  */
  E_ID_IIC_READ,

  /** IIC Address
  * Description: This node sets the IIC control address
  * Data: uint8_t
  * Parent: E_ID_IIC_CMD, E_ID_IIC_REPLY
  */
  E_ID_IIC_ADDR,

  /** IIC Bus Identifier
  * Description: This node sets the IIC control bus
  * Data: uint8_t
  * Parent: E_ID_IIC_CMD, E_ID_IIC_REPLY
  */
  E_ID_IIC_BUS,

  /** IIC Command
  * Description: This node describes an IIC command.
  * If it is a read, the data contains the transfer size. If it is a write, the data contains the content of the transfer.
  * This node can be used several times. One per command
  * Data: uint8_t[]
  * Parent: E_ID_IIC__IGNORE__
  */
  E_ID_IIC_CMD,

  /** IIC REply
  * Description: This node describes an IIC reply. One reply for each byte transfer.
  * This node can be used several times. One per command
  * Data: uint8_t
  * Parent: E_ID_IIC
  */
  E_ID_IIC_REPLY,

  /** IIC Error state
  * Description: This node describes the error state of the transfer
  * Data: uint8_t
  * Parent: E_ID_IIC_REPLY
  */
  E_ID_IIC_ERROR,


  /** IIC Frame Counter
  * Description: This node describes the number of IIC reply received
  * Data: uint8_t
  * Parent: E_ID_IIC_REPLY
  */
  E_ID_IIC_CNT,

  /** Data subnodes.
  * Description: (OBSOLETE)
  * Data sensors.
  * Port: E_PORT_CLIENT (OUT)
  * Data: uint16_t[]
  */
  E_ID_DATA,

  /** Pulse configuration
  * Description: This node describes the generation of a pulse. Pulses are transmitted sequentially in the order of the node's index at each trigger
  * Used by pulser entities.
  * This node can be used several times. One per sequential pulse.
  * This node and all subnodes are taken into account during the update procedure.
  *
  * This node when inserted in an E_ID_PING contains the signal generated at a sampling and demodulation frequency given by E_ID_PULSER_FS_PATTERN and E_ID_PULSER_FS_DEMOD_PATTERN. The format is complex.
  * Data: float2_t[]
  * Parent: E_ID_SCENARIO, E_ID_PING_INFO
  */
  E_ID_PULSE,

  /** Pulse type
  * Description: This node describes the type of the generated pulse
  * Used by pulser entities
  * Data: uint8_t
  * - E_PULSE_OFF = 0, (OBSOLETE)
  * - E_PULSE_LFM, (OBSOLETE)
  * - E_PULSE_HFM, (Hyperbolic FM Pulse)
  * - E_PULSE_CW, (CW Pulse)
  * - E_PULSE_BPSK, (DEV IN PROGRESS)
  *
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_MODE,

  /** Pulse hash config.
  * Value of the pulser hash property.
  * - Port: E_PORT_CLIENT (IN)
  * - Data: uint32 (Nombre)
  * - Default: N/A
  * - Childs: >=0
  * - Quantity: >=1
  */
  E_ID_PULSER_HASH,

  /** Pulse start frequency.
  * Description: This node describes the start frequency of the pulse.
  * Used by pulser entities
  * Data: float (Hz)
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_FSTART,

  /** Pulse end frequency.
  * Description: This node describes the end frequency of the pulse.
  * Used by pulser entities
  * Data: float (Hz)
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_FEND,

  /** Pulse frequency sampling.
  * Description:  This node describes the frequency of the generated signal in the node E_ID_PING_INFO
  * Used by pulser entities
  * Data: float (Hz)
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_FS_PATTERN,

  /** Pulse frequency demodulation sampling.
  * Description:  This node describes the frequency demodulation of the generated signal in the node E_ID_PING_INFO
  * Used by pulser entities
  * Data: float (Hz)
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_FS_DEMOD_PATTERN,

  /** Pulse duration.
  * Description: Pulse duration in second (s).
  * Used by pulser entities
  * Data: double (s)
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_DURATION,

  /** Posterior trigger. (NOT WORKING YET)
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_POST_TRIGGER,

  /** Power amplifier enable.
  * Description: Enable or disable the power amplifier
  * Used by pulser entities
  * Data: uint8_t (Enum)
  * - 0: PA disabled,
  * - 1: PA enabled.
  * Parent: E_ID_SCENARIO
  */
  E_ID_PULSER_PA_ENABLE,

  /** Dead Time.
  * Description: Dead time between top and bottom conduction and vice versa
  * Used by pulser entities
  * Data: uint16_t (Number of HW module clock)
  * Parent: E_ID_SCENARIO
  */
  E_ID_PULSER_DEAD_TIME,

  /** Amplitude to aperture time table.
  * Description:  In the last step of the pulse energy calculation, the module converts the amplitude into aperture-time.
  * Input and output are coded on 10 bits.
  * Used by pulser entities
  * Data: uint16_t[1024] (array)
  *        1.0 = 1023 (max opening time)
  *        0.0 = 0 (min opening time)
  * Default table is : (asin(double(M_MIN(i,i_max_input)) div double(i_max_input)) div M_PI) * 2.0 * double(i_max_output))
  * Max input: 1020 (Max is fix((fix((fix(1023*1023 div 1024)*1023) div 1024)*1023) div 1024)) (Constant pulse amplitude * Sensor weighting * Time weighting * Frequency weighting)
  * Max output: 1023
  * Parent: E_ID_SCENARIO
  */
  E_ID_PULSER_AMP2TIME,


  /** Pulser sensor delays.
  * Description:  This table allows the application of a TX beam forming.
  * Delays are coded in 32 bits. One value per pwm output
  * Used by pulser entities
  * Data: uint16_t[]
  *      Number of Hw clocks per sensor.
  *      Default: 0
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_DELAYS,

  /** Pulser phase adjustment from period.
  * Description:  This table allows the compensation of the phase according to the instantaneous period.
  * Output are coded on 10 bits. Input are coded on 11 bits. Input is half period in Number of Hw clocks.
  * Used by pulser entities
  * Data: int16 (array)
  *      Number of Hw clocks of shift
  *      Default: 0
  * Parent: E_ID_SCENARIO
  */
  E_ID_PULSER_FREQ_WEIGHTING_PHASE,

  /** Pulser amplitude adjustment from period.
  * Description: This table allows the compensation of the amplitude according to the instantaneous period.
  * Input and output are coded on 10 bits. Input are coded on 11 bits. Input is half period in Number of Hw clocks.
  * Used by pulser entities
  * Data: uint16 (array)
  *        1023 = 1.0 (max opening time)
  *        0 = 0.0 (min opening time)
  *        Default: 1023
  * Parent: E_ID_SCENARIO
  */
  E_ID_PULSER_FREQ_WEIGHTING_AMP,

  /** Pulser time amplitude weighting.
  * Description: A time envelope can be applied to each pulse. This node describes the envelope per segment and the step.
  * A fixed number of tables is possible per module (ex: 4)
  * Used by pulser entities
  * Data: uint16 (array)
  *        1.0 = 1023
  *        0.0 = 0
  *        Default: 1023
  * Parent: E_ID_SCENARIO
  */
  E_ID_PULSER_TIME_WEIGHTING,
  /** Pulser time amplitude weighting id.
  * Description: Describes the index of the table used for storing or applying the time envelope.
  * Used by pulser entities. Scenario node only
  * Data: uint8_t
  * Parent: E_ID_PULSER_TIME_WEIGHTING, E_ID_PULSE
  */
  E_ID_PULSER_TIME_WEIGHTING_ID,

  /** Pulser time amplitude weighting step.
  * Description: Describes the interval to go from one index to another in the table describing the signal envelope
  * Used by pulser entities. Scenario node only
  * Data: float (s)
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_TIME_WEIGHTING_STEP,


  /** Sample Offset.
  * Description: The data stream associated with a ping can be split into several sub-pings. These sub-pings are called rings. This node describes the number of the first sample in this ring.
  * Data: uint32_t
  * Parent: E_ID_PING_INFO
  */
  E_ID_SAMPLE_OFFSET,

  /** Ping info.
  * Description: This node gathers all the information from a ping.
  * Data: None
  * Parent: E_ID_PING
  */
  E_ID_PING_INFO,


  /** Scale.
  * Description: This node indicates the coefficient to be applied to the data to obtain Volts
  * Data: double
  * Parent: E_ID_PING_INFO
  */
  E_ID_SCALE,

  /** Format.
  * Description: This node indicates the basic data encoding format
  * Data: uint8_t
  * - E_FORMAT_INT8 = 0,
  * - E_FORMAT_INT16,
  * - E_FORMAT_INT32,
  * - E_FORMAT_INT64,
  * - E_FORMAT_FLOAT,
  * - E_FORMAT_DOUBLE,

  * Parent: E_ID_PING_INFO
  */
  E_ID_FORMAT,

  /** Format.
  * Description: This node indicates the complex data encoding format
  * Data: uint8_t
  * - E_COMPLEX_NONE = 0, (Real value)
  * - E_COMPLEX_FORMAT_TIME_INTERLEAVED, (IQIQIQIQIQIQIQ)
  * - E_COMPLEX_FORMAT_BEAM_INTERLEAVED, (IIIIIIIII, then QQQQQQQQ)
  *
  * Parent: E_ID_PING_INFO
  */
  E_ID_COMPLEX,

  /** System UID.
  * Description: This node indicates the number of the system that generates the parent node
  * Data: uint32_t
  * Parent: E_ID_PING_INFO, E_ID_IIC, E_ID_PING
  */
  E_ID_SYSTEM_UID,

  /** External time.
  * Description: This node indicates the zda time at the time of the trigger.
  * Time converted from the content of the ZDA message. Expressed in hexadecimal.
  * 0x826450020220215 => 0x8 26 45 00 2022 02 15 => 08:26:45 2022/02/15
  * Data: uint64_t
  * Parent: E_ID_HW_UID
  */
  E_ID_EXT_TIME_RAW,

  /** External time.
  * Description: This node indicates the zda time at the time of the trigger.
  * Time converted from the content of the ZDA message. Expressed in clock pulses
  * Data: uint64_t
  * Parent: E_ID_HW_UID
  */
  E_ID_EXT_TIME,

  /** Variable Gain Amplifier value.
  * Description: This node indicates the gain value of the VGA (Variable Gain Amplifier) if not dynamic
  * Data: double (dB)
  * Parent: E_ID_PING_INFO
  */
  E_ID_GAIN_VGA,

  /** Post Gain Amplifier value.
  * Description: This node indicates the gain value of the PGA (Post Gain Amplifier)
  * Data: double (dB)
  * Parent: E_ID_PING_INFO
  */
  E_ID_GAIN_PGA,

  /** LNA Gain value.
  * Description: This node indicates the gain value of the LNA (Low Noise Amplifier)
  * Data: double (dB)
  * Parent: E_ID_PING_INFO
  */
  E_ID_GAIN_LNA,


  /** Total Analog Gain value.
  * Description: This node indicates the gain value of the global gain
  * Data: double (dB)
  * Parent: E_ID_PING_INFO
  */
  E_ID_GAIN,

  /** Missing Samples.
  * Description: This node indicates the number of missing samples.
  * Data: int32_t
  * Parent: E_ID_PING_INFO
  */
  E_ID_SAMPLE_MISSING,

  /** Error Samples.
  * Description: This node shows the data holes in number of samples
  * Data: int32_t
  * Parent: E_ID_PING_INFO
  */
  E_ID_SAMPLE_ERROR,

  /** Number of beam.
  * Description: This node indicates the number of channels.
  * Data: uint32_t
  * Parent: E_ID_PING_INFO
  */
  E_ID_NB_BEAM,

  /** External time from PPS.
  * Description: This node indicates the number of clock counts of the module since the last PPS was received
  * Time since the last PPS signal was received. Measured in clock pulses.
  * Data: uint64_t
  * Parent: E_ID_HW_UID
  */
  E_ID_EXT_TIME_FROM_PPS,

  /** External time from ZDA.
  * Description: This node indicates the number of clock counts of the module since the last ZDA was received.
  * Time since the last ZDA message was received. Measured in clock pulses (125 MHz) rounded to 2**16 (0.5 ms)
  * Data: uint64_t
  * Parent: E_ID_HW_UID
  */
  E_ID_EXT_TIME_FROM_ZDA,

  /** External time from ZDA.
  * Description: This node indicates the number of clock counts of the module since the last ZDA was received when PPS has been received.
  * The time between the arrival of the ZDA message and PPS signal. Expressed in clock pulses counting from the ZDA message decoded till the receipt of the PPS signal
  * Data: uint64_t
  * Parent: E_ID_HW_UID
  */
  E_ID_EXT_TIME_FROM_ZDA_AT_PPS,

 /** External ping time.
  * Description: This node indicates the ping time in nanosec at the time of the trigger.
  * Time calculated from the received ZDA (E_ID_EXT_TIME) and the field E_ID_EXT_TIME_FROM_PPS. This time is expressed in ns from 1 jan 1970
  * Data: uint64_t
  * Parent: E_ID_HW_UID
  */
  E_ID_EXT_TIME_WITH_FROM_PPS,

  /** Trigger UID.
  * Description: This node indicates the unique identifier of the trigger.
  * This identifier is composed of the trigger UID and the ring ID
  * Data: uint32_t
  * Parent: E_ID_HW_UID
  */
  E_ID_TRIGGER_UID,

#if 0
  /** Trigger UID.
  * Description: This node indicates the unique identifier of the trigger.
  * This identifier is composed of the trigger UID and the ring ID
  * Data: uint64_t
  * Parent: E_ID_PING
  */
  E_ID_TRIGGER_UID__IGNORE__,
#endif

  /** Jtag debug trigger.
  * Description: This node triggers the reading of the JTAG stream if enabled at compile time.
  * Data: None
  * Port: E_PORT_CLIENT (IN)
  */
  E_ID_JTAG_TRIGGER,

  /** Jtag debug trigger.
  * Description:This node configures the JTAG link if compiled.
  * Data: uint16_t
  * The data indicates the number of seconds to wait between the GPIO ignition and the sending of the initialization frames
  * Port: E_PORT_CLIENT (IN)
  */
  E_ID_JTAG_INIT,

  /** Pulse amplitude.
  * Description: This value is multiplied by several weights to obtain the opening time.
  * Used by pulser entities
  * Data: double
  * - Min = 0.0
  * - Max = 1.0
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_AMPLITUDE,

  /** Pulse delay.
  * Description: This value is the delay (in seconds) from start of Rx to start of Tx.
  * Value is 0 if starts of Rx and Tx are synchronized.
  * Used by pulser entities
  * Data: single
  * - Min = 0.0
  * - Max = Rx duration - Pulse duration
  * Parent: E_ID_PULSE
  */
  E_ID_PULSER_DELAY,

  /** Terminaison Mode.
  * Description:  This node indicates the termination mode of the ADCs. (only AD9670)
  * Used by ADC entities
  * Data: uint8_t
  * - E_ADC_TERM_MODE_Z1 = 0,
  * - E_ADC_TERM_MODE_Z2,
  * - E_ADC_TERM_MODE_Z1_Z2,
  * - E_ADC_TERM_MODE_OPEN,
  * Parent: E_ID_SCENARIO
  */
  E_ID_TERM_MODE,

  /** Acquisition Mode.
  * Description:  This node indicates how the hw will stop after a trigger.
  * - E_ADC_ACQ_MODE_WINDOWED (default) : It stops after E_ID_NB_SAMPLE samples is reached.
  * - E_ADC_ACQ_MODE_CONTINUOUS (default) : It never stops. Plugins send data per block of E_ID_NB_SAMPLE samples.
  * Data: uint8_t
  * - E_ADC_ACQ_MODE_WINDOWED = 0,
  * - E_ADC_ACQ_MODE_CONTINUOUS
  * Parent: E_ID_SCENARIO
  */
  E_ID_ACQ_MODE,

  /** Doppler sensitivity.
  * Description:  This node indicates the doppler sensitivity of the pulse
  * Used by ADC entities
  * Data: uint8_t
  * - 1 : Pulse is doppler sensitive
  * - 0 : Pulse is not doppler sensitive
  * Parent: E_ID_PULSE
  */
  E_ID_DOPPLER_SENSITIVE,

  /** Number of Doppler copies per radial speed (m/s).
  * Description:  This node indicates the number of doppler copies requiered per m/s
  * Used by Processing entities
  * Data: float
  * - 0 : No copies requiered
  * - other : The number of copies is (speed range) * (value)
  * Parent: E_ID_PULSE
  */
  E_ID_NB_CP_DOPP_M_S,

  /** User Id of configuration.
  * Description: This node is not used by the plugin, but it is passed to the data stream to allow the identification of a configuration
  * Used by ADC entities
  * Data: uint16_t
  * Parent: E_ID_SCENARIO, E_ID_HW_UID
  */
  E_ID_USER_CONFIG,

  /** Trigger error offset.
  * Description: This node indicates the number of trigger that seems to be missed by the module
  * Used by ADC entities
  * Data: uint32_t
  * Parent: E_ID_HW_UID
  */
  E_ID_TRIGGER_ERROR_OFFSET,

  /** trigger date.
  * trigger date in system time.
  * - Port: E_PORT_CLIENT (IN)
  * - Data: uint64 (nombre)
  * - Default: N/A
  * - Childs: None
  * - Quantity: 1
  */
  E_ID_TIME,

  /** Ring ID of ping. (OBSOLETE)
  */
  E_ID_RING_ID,

  /** Error Samples ratio.
  * Description: This node shows the data holes in a ratio format
  * Data: float
  * Parent: E_ID_PING_INFO
  */
  E_ID_SAMPLE_LOST_RATE,


  /** DAC control
  * Description:  This node describes the operation of the DAC entities
  * Used by DAC entities
  * Data: None
  * Parent: E_ID_SCENARIO
  */
  E_ID_DAC,

  /** DAC step
  * Description:  This node describes the interval in number of ADC samples between two values of the DAC data tables.
  * Used by DAC entities
  * Data: uint16_t
  * Parent: E_ID_DAC
  */
  E_ID_DAC_STEP,

  /** DAC table No 1
  * Description: This node describe the output value of DAC1 per sample step.
  * Used by DAC entities
  * Data: uint16_t[]
  * Parent: E_ID_DAC
  */
  E_ID_DAC_VGA1,

  /** DAC table No 2
  * Description: This node describe the output value of DAC2 per sample step.
  * Used by DAC entities
  * Data: uint16_t[]
  * Parent: E_ID_DAC
  */
  E_ID_DAC_VGA2,


  /** Broadcast ID.
  * Description: This node is broadcast on the HW port, so that a potential switch connected to this port does not lose the assignment between the mac address and the switch port
  * Data: None
  * Port: E_PORT_HW(OUT)
  */
  E_ID_BROADCAST,


  /** Hardware generale purpose output.
  * Description: hardware purpose output content
  * Data: uint8_t
  * Parent: E_ID_SCENARIO
  */
  E_ID_HW_GPO,

  /** Licencing status.
  * Description: thi flag indicate the validity of the licence
  * Data: uint8_t
  * - 0: means invalid
  * - 1: means valid
  * Parent: E_ID_READY
  */
  E_ID_LICENCE,
};
#endif

#define E_IDHW_RTAM_BASE 0xCAE0

/***********************************************************************
 * Defines
 ***********************************************************************/

enum ET_SYST_TYPE {
  E_SYST_TYPE_SYSTEM = 0,
  E_SYST_TYPE_ADC,
  E_SYST_TYPE_ADC_SPI,
  E_SYST_TYPE_PULSER,
  E_SYST_TYPE_IIC,
  E_SYST_TYPE_DAC,
};

enum ET_CHANNELS_MODE {
  E_CHANNELS_BLACK_LIST = 0, E_CHANNELS_WHITE_LIST,
};

enum ET_MUX {
  E_ADC_MUX_INPUT = 0, E_ADC_MUX_DEMOD,  E_MUX_FIR, E_MUX_DECIM, E_MUX_NB_MUX,
};

enum ET_FORMAT {
  E_FORMAT_INT8 = 0, E_FORMAT_INT16, E_FORMAT_INT32, E_FORMAT_INT64, E_FORMAT_FLOAT, E_FORMAT_DOUBLE,
};
enum ET_COMPLEX {
  E_COMPLEX_NONE = 0, E_COMPLEX_FORMAT_TIME_INTERLEAVED, E_COMPLEX_FORMAT_BEAM_INTERLEAVED,
};
enum ET_TRIGGER_MODE {
  E_TRIGGER_EXTERNAL = 0, E_TRIGGER_ACQUISITION_START, E_TRIGGER_NB_MODES
};

enum ET_TRIGGER_LATCH_MODE {
  E_TRIGGER_LATCH_MODE_NONE = 0, E_TRIGGER_LATCH_MODE_AC, E_TRIGGER_LATCH_MODE_FALLING, E_TRIGGER_LATCH_NB_MODES
};

enum ET_PPS_EDGE_MODE {
  E_PPS_EDGE_MODE_RISING = 0, E_PPS_EDGE_MODE_FALLING
};

enum ET_ADC_TEST_MODE {
  E_ADC_TEST_MODE_DISABLE = 0,
  E_ADC_TEST_MODE_SINE,
  E_ADC_TEST_MODE_DECIMATOR_FILTER,
  E_ADC_TEST_MODE_CHANNEL_ID,
  E_ADC_TEST_MODE_ANALOG_TONE,
  E_ADC_TEST_MODE_USER_INPUT,
  E_ADC_TEST_MODE_DECIMATOR_FILTER_FPGA,
  E_ADC_TEST_MODE_DECIMATOR_NO_ACCUMULATOR_GAIN,
  E_ADC_TEST_MODE_ADC_ID,
  E_ADC_TEST_MODE_PRN_SHORT,
  E_ADC_TEST_MODE_PRN_LONG,
  E_ADC_TEST_MODE_NB_TESTS,
};

enum ET_ADC_TERM_MODE {
  E_ADC_TERM_MODE_Z1 = 0,
  E_ADC_TERM_MODE_Z2,
  E_ADC_TERM_MODE_Z1_Z2,
  E_ADC_TERM_MODE_OPEN,
  E_ADC_TERM_MODE_NB
};

enum ET_ADC_ACQ_MODE {
  E_ADC_ACQ_MODE_WINDOWED = 0,
  E_ADC_ACQ_MODE_CONTINUOUS,
  E_ADC_ACQ_MODE_NB
};

enum ET_PULSE_MODE {
  E_PULSE_OFF = 0,
  E_PULSE_LFM,
  E_PULSE_HFM,
  E_PULSE_CW,
  E_PULSE_BPSK,
  E_PULSE_NB_MODES
};

enum ET_TASK {
  E_TASK_INTEGRATOR = 0,
  E_TASK_MAX_NB,
};



}
}
}
#endif /* RTAM_EVENT_HH_ */
