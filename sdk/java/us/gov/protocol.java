package us.gov;

public class protocol {
    public static final short protocol_suffix=0;
    public static boolean is_node_protocol(short i) { return (i&3)==protocol_suffix; }

//---------------------------------------------------------------------generated by make, do not edit
//content of file: ../../api/apitool_generated__protocol_gov_socket_java
//------------------generated by apitool- do not edit
// gov_socket - master file: us/apitool/data/gov_socket
    public static final short gov_socket_base = 100;
    public static final short gov_socket_error = ((gov_socket_base+0)<<2)+protocol_suffix;
    public static final short gov_socket_ping = ((gov_socket_base+1)<<2)+protocol_suffix;
    public static final short gov_socket_pong = ((gov_socket_base+2)<<2)+protocol_suffix;
//-/----------------generated by apitool- do not edit

//content of file: ../../api/apitool_generated__protocol_gov_id_java
//------------------generated by apitool- do not edit
// gov_id - master file: us/apitool/data/gov_id
    public static final short gov_id_base = 200;
    public static final short gov_id_request = ((gov_id_base+0)<<2)+protocol_suffix;
    public static final short gov_id_peer_challenge = ((gov_id_base+1)<<2)+protocol_suffix;
    public static final short gov_id_challenge_response = ((gov_id_base+2)<<2)+protocol_suffix;
//-/----------------generated by apitool- do not edit

//-/-------------------------------------------------------------------generated by make, do not edit



}

