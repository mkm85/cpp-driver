/*
  Copyright (c) 2014-2016 DataStax

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#ifndef __CCM_BRIDGE_HPP__
#define __CCM_BRIDGE_HPP__

#include "authentication_type.hpp"
#include "bridge_exception.hpp"
#include "cass_version.hpp"
#include "deployment_type.hpp"
#include "dse_credentials_type.hpp"
#include "socket.hpp"

#include <map>
#include <string>
#include <vector>

#ifdef CASS_USE_LIBSSH2
// Forward declarations for libssh2
typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;
typedef struct _LIBSSH2_CHANNEL LIBSSH2_CHANNEL;
#endif

#if defined(_MSC_VER)
#  define CCM_BRIDGE_DEPRECATED(func) __declspec(deprecated) func
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
#  define CCM_BRIDGE_DEPRECATED(func) func __attribute__((deprecated))
#else
#  define CCM_BRIDGE_DEPRECATED(func) func
#endif

// Default values
#define DEFAULT_CASSANDRA_VERSION CassVersion("3.4")
#define DEFAULT_DSE_VERSION DseVersion("4.8.5")
#define DEFAULT_USE_GIT false
#define DEFAULT_USE_DSE false
#define DEFAULT_CLUSTER_PREFIX "cpp-driver"
#define DEFAULT_DSE_CREDENTIALS DseCredentialsType::USERNAME_PASSWORD
#define DEFAULT_DEPLOYMENT DeploymentType::LOCAL
#define DEFAULT_AUTHENTICATION AuthenticationType::USERNAME_PASSWORD
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_REMOTE_DEPLOYMENT_PORT 22
#define DEFAULT_REMOTE_DEPLOYMENT_USERNAME "vagrant"
#define DEFAULT_REMOTE_DEPLOYMENT_PASSWORD "vagrant"
#define DEFAULT_JVM_ARGUMENTS std::vector<std::string>()

// Define the node limit for a cluster
#define CLUSTER_NODE_LIMIT 6

/**
 * Node status information for a cluster
 */
struct ClusterStatus {
  /**
   * List of IPv4 addresses for `DECOMMISSIONED` nodes
   */
  std::vector<std::string> nodes_decommissioned;
  /**
   * List of IPv4 addresses for `DOWN` or unavailable nodes
   */
  std::vector<std::string> nodes_down;
  /**
   * List of IPv4 addresses for `uninitialized` nodes
   */
  std::vector<std::string> nodes_uninitialized;
  /**
   * List of IPv4 addresses for `UP` or ready nodes
   */
  std::vector<std::string> nodes_up;
  /**
   * Total number of nodes in the cluster
   */
  unsigned int node_count;

  /**
   * Constructor
   */
  ClusterStatus() : node_count(0) {}
};

namespace CCM {

  class Bridge {
  public:
    /**
     * Constructor
     *
     * @param cassandra_version Cassandra version to use
     *                          (default: DEFAULT_CASSANDRA_VERSION)
     * @param use_git True if version should be obtained from ASF/GitHub; false
     *                otherwise (default: DEFAULT_USE_GIT). Prepends
     *                `cassandra-` to version when creating cluster through CCM
     *                if using Cassandra; otherwise passes version information
     *                to CCM for git download of DSE.
     * @param use_dse True if CCM should load DSE for provided version; false
     *               otherwise (default: DEFAULT_USE_DSE)
     * @param cluster_prefix Prefix to use when creating a cluster name
     *                       (default: DEFAULT_CLUSTER_PREFIX)
     * @param dse_credentials_type Username|Password/INI file credentials
     *                             (default: DEFAULT_DSE_CREDENTIALS)
     * @param dse_username Username for DSE download authentication; empty if
     *                     using INI file credentials (default: Empty)
     * @param dse_password Password for DSE download authentication; empty if
     *                     using INI file credentials (default: Empty)
     * @param deployment_type Local|Remote deployment (execute command locally
     *                        or remote (default: DEFAULT_DEPLOYMENT)
     * @param authentication_type Username|Password/Public key authentication
     *                            (default: DEFAULT_AUTHENTICATION)
     * @param host Host/IP address for CCM cluster and/or SSH connection
     *             (default: DEFAULT_HOST)
     * @param port TCP/IP port for SSH connection
     *             (default: DEFAULT_REMOTE_DEPLOYMENT_PORT)
     * @param username Username for SSH authentication
     *                 (default: DEFAULT_REMOTE_DEPLOYMENT_USERNAME)
     * @param password Password for SSH authentication; Empty if using public
     *                 key (default: DEFAULT_REMOTE_DEPLOYMENT_PASSWORD)
     * @param public_key Public key for authentication; Empty if using username
     *                   and password authentication (default: Empty)
     * @param private_key Private key for authentication; Empty if using
     *                   username and password authentication (default: Empty)
     * @throws BridgeException
     */
    Bridge(CassVersion cassandra_version = DEFAULT_CASSANDRA_VERSION,
      bool use_git = DEFAULT_USE_GIT,
      bool use_dse = DEFAULT_USE_DSE,
      const std::string& cluster_prefix = DEFAULT_CLUSTER_PREFIX,
      DseCredentialsType dse_credentials_type = DEFAULT_DSE_CREDENTIALS,
      const std::string& dse_username = "",
      const std::string& dse_password = "",
      DeploymentType deployment_type = DEFAULT_DEPLOYMENT,
      AuthenticationType authentication_type = DEFAULT_AUTHENTICATION,
      const std::string& host = DEFAULT_HOST,
      short port = DEFAULT_REMOTE_DEPLOYMENT_PORT,
      const std::string& username = DEFAULT_REMOTE_DEPLOYMENT_USERNAME,
      const std::string& password = DEFAULT_REMOTE_DEPLOYMENT_PASSWORD,
      const std::string& public_key = "",
      const std::string& private_key = "");

    /**
     * Constructor
     *
     * @param configuration_file Full path to configuration file
     * @throws BridgeException
     * @deprecated Configuration file will be removed after new test framework
     *             is fully implemented
     */
    CCM_BRIDGE_DEPRECATED(Bridge(const std::string& configuration_file));

    /**
     * Destructor
     */
    ~Bridge();

    /**
     * Clear the data on the active cluster; as a side effect the cluster is
     * also stopped
     */
    void clear_cluster_data();

    /**
     * Get a comma separated list of IPv4 addresses for nodes in the active
     * Cassandra cluster
     *
     * @param is_all True if all node IPv4 addresses should be returned; false
     *               if only the `UP` nodes (default: true)
     * @return Comma separated list of IPv4 addresses
     */
    std::string cluster_contact_points(bool is_all = true);

    /**
     * Get the list of IPv4 addresses for node in the active Cassandra cluster
     *
     * @param is_all True if all node IPv4 addresses should be returned; false
     *               if only the `UP` nodes (default: true)
     * @return Array/Vector of IPv4 addresses
     */
    std::vector<std::string> cluster_ip_addresses(bool is_all = true);

    /**
     * Get the status for the active cluster
     *
     * @return Status of the nodes in the active cluster
     */
    ClusterStatus cluster_status();

    /**
     * Create a Cassandra cluster
     *
     * @param data_center_one_nodes Number of nodes for DC1 (default: 1)
     * @param data_center_two_nodes Number of nodes for DC2 (default: 0)
     * @param is_ssl True if SSL should be enabled; false otherwise
     *               (default: false)
     * @param is_client_authentication True if client authentication should be
     *                                enabled; false otherwise (default: false)
     * @return True if cluster was created or switched; false otherwise
     */
    bool create_cluster(unsigned short data_center_one_nodes = 1,
      unsigned short data_center_two_node = 0,
      bool is_ssl = false, bool is_client_authentication = false);

    /**
     * Check to see if the active cluster is no longer accepting connections
     *
     * NOTE: This method may check the status of the nodes in the cluster
     *       multiple times
     *
     * @return True if cluster is no longer accepting connections; false
     *    otherwise
     */
    bool is_cluster_down();

    /**
     * Check to see if the active cluster is ready to accept connections
     *
     * NOTE: This method may check the status of the nodes in the cluster
     *       multiple times
     *
     * @return True if cluster is ready to accept connections; false otherwise
     */
    bool is_cluster_up();

    /**
     * Alias for stop_cluster(true)
     */
    bool kill_cluster();

    /**
     * Remove active cluster
     */
    void remove_cluster();

    /**
     * Remove a cluster
     *
     * @param cluster_name Cluster name to remove
     */
    void remove_cluster(const std::string& cluster_name);

    /**
     * Remove all the available clusters
     * (default deletes generated bridge clusters)
     *
     * @param is_all If true all CCM clusters are removed; otherwise false to
     *               delete bridge generated clusters (default: false)
     */
    void remove_all_clusters(bool is_all = false);

    /**
     * Start the active Cassandra cluster
     *
     * @param jvm_arguments Array/Vector of JVM arguments to apply during
     *                     cluster start
     * @return True if cluster is up; false otherwise
     */
    bool start_cluster(std::vector<std::string> jvm_arguments);

    /**
     * Start the active Cassandra cluster
     *
     * @param jvm_argument JVM argument to apply during cluster start (optional)
     * @return True if cluster is down; false otherwise
     */
    bool start_cluster(std::string jvm_argument = "");

    /**
     * Stop the active Cassandra cluster
     *
     * @param is_kill True if forced termination requested; false otherwise
     *                (default: false)
     * @return True if cluster is down; false otherwise
     */
    bool stop_cluster(bool is_kill = false);

    /**
     * Switch to another available cluster
     *
     * @param cluster_name Cluster name to switch to
     * @return True if switched or is currently active cluster; false otherwise
     */
    bool switch_cluster(const std::string& cluster_name);

    /**
     * Update the cluster configuration
     *
     * @param key_value_pairs Key:Value to update
     * @param is_dse True if key/value pair should update the dse.yaml file;
     *               otherwise false (default: false)
     */
    void update_cluster_configuration(std::vector<std::string> key_value_pairs, bool is_dse = false);

    /**
     * Update the cluster configuration
     *
     * @param key Key to update
     * @param value Value to apply to key configuration
     * @param is_dse True if key/value pair should update the dse.yaml file;
     *               otherwise false (default: false)
     */
    void update_cluster_configuration(const std::string& key, const std::string& value, bool is_dse = false);

    /**
     * Add a node on the active Cassandra cluster
     *
     * @param data_center If provided add the node to the data center; otherwise
     *                    add node normally (default: no data center)
     * @return Node added to cluster
     * @throws BridgeException
     */
    unsigned int add_node(const std::string& data_center = "");

    /**
     * Bootstrap (add and start) a node on the active Cassandra cluster
     *
     * @param jvm_argument JVM argument to apply during node start
     *                     (default: no JVM argument
     * @param data_center If provided add the node to the data center; otherwise
     *                    add node normally (default: no data center)
     * @return Node added to cluster
     * @throws BridgeException
     */
    unsigned int bootstrap_node(const std::string& jvm_argument = "", const std::string& data_center = "");

    /**
     * Decommission a node on the active Cassandra cluster
     *
     * @param node Node to decommission
     * @return True if node was decommissioned; false otherwise
     */
    bool decommission_node(unsigned int node);

    /**
     * Disable binary protocol for a node on the active Cassandra cluster
     *
     * @param node Node to disable binary protocol
     */
    void disable_node_binary_protocol(unsigned int node);

    /**
     * Disable gossip for a node on the active Cassandra cluster
     *
     * @param node Node to disable gossip
     */
    void disable_node_gossip(unsigned int node);

    /**
     * Enable binary protocol for a node on the active Cassandra cluster
     *
     * @param node Node to enable binary protocol
     */
    void enable_node_binary_protocol(unsigned int node);

    /**
     * Enable gossip for a node on the active Cassandra cluster
     *
     * @param node Node to enable gossip
     */
    void enable_node_gossip(unsigned int node);

    /**
     * Execute a CQL statement on a particular node
     *
     * @param node Node to execute CQL statement on
     * @param cql CQL statement to execute
     */
    void execute_cql_on_node(unsigned int node, const std::string& cql);

    /**
     * Alias for stop_node(node, true)
     *
     * @return True if node is down; false otherwise
     */
    bool kill_node(unsigned int node);

    /**
     * Pause a node on the active Cassandra cluster
     *
     * @param node Node to pause
     */
    void pause_node(unsigned int node);

    /**
     * Resume a node on the active Cassandra cluster
     *
     * @param node Node to resume
     */
    void resume_node(unsigned int node);

    /**
     * Start a node on the active Cassandra cluster
     *
     * @param node Node to start
     * @param jvm_arguments Array/Vector of JVM arguments to apply during node
     *                      start (default: DEFAULT_JVM_ARGUMENTS)
     * @return True if node is up; false otherwise
     */
    bool start_node(unsigned int node, std::vector<std::string> jvm_arguments = DEFAULT_JVM_ARGUMENTS);

    /**
     * Start a node on the active Cassandra cluster with an additional JVM
     * argument
     *
     * @param node Node to start
     * @param jvm_argument JVM argument to apply during node start
     * @return True if node is up; false otherwise
     */
    bool start_node(unsigned int node, std::string jvm_argument);

    /**
     * Stop a node on the active Cassandra cluster
     *
     * @param node Node to stop
     * @param is_kill True if forced termination requested; false otherwise
     *                (default: false)
     * @return True if node is down; false otherwise
     */
    bool stop_node(unsigned int node, bool is_kill = false);

    /**
     * Get the IP address prefix from the host IP address
     *
     * @return IP address prefix
     */
    std::string get_ip_prefix();

    /**
     * Get the IP prefix indicated by the `HOST` setting in the configuration
     * file
     *
     * @param configuration_file Full path to configuration file
     * @return IP prefix from the configuration file
     * @deprecated Configuration file will be removed after new test framework
     *             is fully implemented
     */
    CCM_BRIDGE_DEPRECATED(static std::string get_ip_prefix(const std::string& configuration_file));

    /**
     * Get the Cassandra version from the active cluster
     *
     * @return Cassandra version
     * @throws BridgeException
     */
    CassVersion get_cassandra_version();

    /**
     * Get the Cassandra version indicated in the configuration file
     *
     * @param configuration_file Full path to configuration file
     * @return Cassandra version from the configuration file
     * @deprecated Configuration file will be removed after new test framework
     *             is fully implemented
     */
    CCM_BRIDGE_DEPRECATED(static CassVersion get_cassandra_version(const std::string& configuration_file));

    /**
     * Get the DSE version from the active cluster
     *
     * @return DSE version
     * @throws BridgeException
     */
    DseVersion get_dse_version();

    /**
     * Get the DSE version indicated in the configuration file
     *
     * @param configuration_file Full path to configuration file
     * @return DSE version from the configuration file
     * @deprecated Configuration file will be removed after new test framework
     *             is fully implemented
     */
    CCM_BRIDGE_DEPRECATED(static DseVersion get_dse_version(const std::string& configuration_file));

    /**
     * Check to see if a node has been decommissioned
     *
     * @param node Node to check `DECOMMISSION` status
     * @return True if node is decommissioned; false otherwise
     */
    bool is_node_decommissioned(unsigned int node);

    /**
     * Check to see if a node will no longer accept connections
     *
     * NOTE: This method may check the status of the node multiple times
     *
     * @param node Node to check `DOWN` status
     * @return True if node is no longer accepting connections; false otherwise
     */
    bool is_node_down(unsigned int node);

    /**
     * Check to see if a node is ready to accept connections
     *
     * NOTE: This method may check the status of the node multiple times
     *
     * @param node Node to check `UP` status
     * @return True if node is ready to accept connections; false otherwise
     */
    bool is_node_up(unsigned int node);

  private:
#ifdef CASS_USE_LIBSSH2
    /**
     * SSH session handle for establishing connection
     */
    LIBSSH2_SESSION* session_;
    /**
     * SSH channel handle for interacting with the session
     */
    LIBSSH2_CHANNEL* channel_;
#endif
    /**
     * Socket instance
     */
    Socket* socket_;
    /**
     * Cassandra version to use
     */
    CassVersion cassandra_version_;
    /**
     * DSE version to use
     */
    DseVersion dse_version_;
    /**
     * Flag to determine if Cassandra/DSE should be built from ASF/GitHub
     */
    bool use_git_;
    /**
     * Flag to determine if DSE is being used
     */
    bool use_dse_;
    /**
     * Cluster prefix to apply to cluster name during create command
     */
    std::string cluster_prefix_;
    /**
     * Deployment type (local|ssh)
     *
     * Flag to indicate how CCM commands should be executed
     */
    DeploymentType deployment_type_;
    /**
     * Authentication type (username|password/public key)
     *
     * Flag to indicate how SSH authentication should be established
     */
    AuthenticationType authentication_type_;
    /**
     * DSE credentials type (username|password/INI file)
     *
     * Flag to indicate how DSE credentials should be obtained
     */
    DseCredentialsType dse_credentials_type_;
    /**
     * Username to use when authenticating download access for DSE
     */
    std::string dse_username_;
    /**
     * Password to use when authenticating download access for DSE
     */
    std::string dse_password_;
    /**
     * IP address to use when establishing SSH connection for remote CCM
     * command execution and/or IP address to use for server connection IP
     * generation
     */
    std::string host_;

#ifdef CASS_USE_LIBSSH2
    /**
     * Initialize the socket
     *
     * @param host Host/IP address for SSH connection
     * @param port TCP/IP port for SSH connection
     * @throws SocketException
     */
    void initialize_socket(const std::string& host, short port);

    /**
     * Synchronize the socket based on the direction of the libssh2 session
     *
     * @throws BridgeException
     * @throws SocketException
     */
    void synchronize_socket();

    /**
     * Initialize the libssh2 connection
     *
     * @throws BridgeException
     */
    void initialize_libssh2();

    /**
     * Establish connection via libssh2
     *
     * @param authentication_type Username|Password/Public key authentication
     * @param username Username for SSH authentication
     * @param password Password for SSH authentication; NULL if using public
     *                 key
     * @param public_key Public key for authentication; Empty if using username
     *                   and password authentication
     * @param private_key Private key for authentication; Empty if using
     *                   username and password authentication
     * @throws BridgeException
     */
    void establish_libssh2_connection(AuthenticationType authentication_type,
      const std::string& username, const std::string& password,
      const std::string& public_key, const std::string& private_key);

    /**
     * Create/Open the libssh2 terminal
     *
     * @throws BridgeException
     */
    void open_libssh2_terminal();

    /**
     * Terminate/Close the libssh2 terminal
     *
     * @throws BridgeException
     */
    void close_libssh2_terminal();

    /**
     * Execute a remote command on the libssh2 connection
     *
     * @param command Command array to execute ([0] = command, [1-n] arguments)
     * @return Output from executed command
     */
    std::string execute_libssh2_command(const std::vector<std::string>& command);
#endif

    /**
     * Execute a local command
     *
     * @param command Command array to execute ([0] = command, [1-n] arguments)
     * @return Output from executed command
     */
    std::string execute_local_command(const std::vector<std::string>& command);

#ifdef CASS_USE_LIBSSH2
    /**
     * Read the output (stdout and stderr) from the libssh2 terminal
     *
     * @return Output from the terminal (may not be in order)
     */
    std::string read_libssh2_terminal();

    /**
     * Finalize the libssh2 library usage and socket used by libssh2
     */
    void finalize_libssh2();
#endif

    /**
     * Execute the CCM command
     *
     * @param command Command array to execute ([0] = CCM command,
     *                [1-n] arguments)
     * @return Output from executing CCM command
     */
    std::string execute_ccm_command(const std::vector<std::string>& command);

    /**
     * Get the active Cassandra cluster
     *
     * @return Currently active Cassandra cluster
     */
    std::string get_active_cluster();

    /**
     * Get the list of available Cassandra clusters
     *
     * @return Array/Vector of available Cassandra clusters
     */
    std::vector<std::string> get_available_clusters();

    /**
     * Get the list of available Cassandra clusters
     *
     * @param [out] active_cluster Current active cluster in the list
     * @return Array/Vector of available Cassandra clusters
     */
    std::vector<std::string> get_available_clusters(std::string& active_cluster);

    /**
     * Generate the name of the Cassandra cluster based on the number of nodes
     * in each data center
     *
     * @param cassandra_version Cassandra version being used
     * @param data_center_one_nodes Number of nodes for DC1
     * @param data_center_two_nodes Number of nodes for DC2
     * @param is_ssl True if SSL is enabled; false otherwise
     * @param is_client_authentiction True if client authentication is enabled;
     *                                false otherwise
     */
    std::string generate_cluster_name(CassVersion cassandra_version,
      unsigned short data_center_one_nodes,
      unsigned short data_center_two_nodes,
      bool is_ssl, bool is_client_authentication);

    /**
     * Generate the nodes parameter for theCassandra cluster based on the number
     * of nodes in each data center
     *
     * @param data_center_one_nodes Number of nodes for DC1
     * #param data_center_two_nodes Number of nodes for DC2
     */
    std::string generate_cluster_nodes(unsigned short data_center_one_nodes, unsigned short data_center_two_nodes);

    /**
     * Generate the CCM update configuration command based on the Cassandra
     * version requested
     *
     * @param cassandra_version Cassandra version to use
     * @return Array/Vector containing the updateconf command
     */
    std::vector<std::string> generate_create_updateconf_command(CassVersion cassandra_version);

    /**
     * Get the next available node
     *
     * @return Next available node
     * @throws BridgeException
     * @see CLUSTER_NODE_LIMIT
     */
    unsigned int get_next_available_node();

    /**
     * Generate the node name based on the node requested
     *
     * @param node Node to use
     * @return Name of the node for CCM node commands
     */
    std::string generate_node_name(unsigned int node);

    /**
     * Determine if a node is available
     *
     * @param node Cassandra node to check
     * @return True if node is available; false otherwise
     */
    bool is_node_availabe(unsigned int node);

    /**
     * Determine if a node is available
     *
     * @param ip_address IPv4 address of the Cassandra node
     * @return True if node is available; false otherwise
     */
    bool is_node_availabe(const std::string& ip_address);

    /**
     * Convert a string to lowercase
     *
     * @param input String to convert to lowercase
     *
     * TODO: Remove static declaration after deprecations are removed
     */
    static std::string to_lower(const std::string& input);

    /**
     * Remove the leading and trailing whitespace from a string
     *
     * @param input String to trim
     * @return Trimmed string
     *
     * TODO: Remove static declaration after deprecations are removed
     */
    static std::string trim(const std::string& input);

    /**
     * Concatenate an array/vector into a string
     *
     * @param elements Array/Vector elements to concatenate
     * @param delimiter Character to use between elements (default: <space>)
     * @return A string representation of all the array/vector elements
     */
    std::string implode(const std::vector<std::string>& elements, const char delimiter = ' ');

    /**
     * Split a string into an array/vector
     *
     * @param input String to convert to array/vector
     * @param delimiter Character to use split into elements (default: <space>)
     * @return An array/vector representation of the string
     *
     * TODO: Remove static declaration after deprecations are removed
     */
    static std::vector<std::string> explode(const std::string& input, const char delimiter = ' ');

    /**
     * Cross platform millisecond granularity sleep
     *
     * @param milliseconds Time in milliseconds to sleep
     */
    void msleep(unsigned int milliseconds);
  };
}

#endif // __CCM_BRIDGE_HPP__
