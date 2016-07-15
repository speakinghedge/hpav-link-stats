import os
import subprocess
import yaml

HPAV_BINARY_PATH = 'build/bin/hpav-stats'

class HPAVStats:
    """
    small wrapper for hpav-stats binary
    """
    def __init__(self, hpav_stats_binary_path):

        self._binary_path = os.path.realpath(hpav_stats_binary_path)

    def link_stats(self, interface, network_id, device_mac):
        """
        request link statistics for given network id and device (MAC)

        example: pprint.pprint(HS.link_stats('eth2', 'XX:XX:XX:XX:82:9e:06', 'f4:06:8d:15:XX:XX'))

        :param interface:  network interface used to communicate with the HPAV network
        :param network_id: to query the link statistics for
        :param device_mac: of the device to get the link stats for
        :return: dictionary containing network statistic entries
        """
        try:
            discover = subprocess.check_output([self._binary_path, '-L', '-n', network_id, '-m', device_mac, interface])
            yml = yaml.load(discover)

            if yml is None:
                raise AttributeError('no data returned from command')

            result = {
                'rx': [],
                'tx': []
            }

            for section in yml:
                if 'transmit_link_stats_' in section:
                    result['tx'].append(yml[section])
                elif 'receive_link_stats_' in section:
                    result['rx'].append(yml[section])
                else:
                    raise KeyError('unknown or invalid section in result: {0}'.format(section))

            return result

        except:
            raise

    def network_stats(self, interface, network_id):
        """
        request network statistics for given network id

        example: pprint.pprint(HS.network_stats('eth2', 'XX:XX:XX:XX:82:9e:06'))

        :param interface:  network interface used to communicate with the HPAV network
        :param network_id: to query the network statistics for
        :return: dictionary containing network statistic entries
        """

        try:
            discover = subprocess.check_output([self._binary_path, '-N', '-n', network_id, interface])
            yml = yaml.load(discover)

            if yml is None:
                raise AttributeError('no data returned from command')

            result = {}

            for report in yml:
                if 'network_stats_' in report:
                    osa = yml[report]['osa']
                    del yml[report]['osa']
                    if osa not in result:
                        result[osa] = [yml[report]]
                    else:
                        result[osa].append(yml[report])
                else:
                    raise KeyError('unknown or invalid report in result: {0}'.format(report))

            return result

        except:
            raise


def print_link_stats(mac_a, stats_a, mac_b, stats_b):

    print '{0} ( <-> {1})'.format(mac_a, mac_b)
    for entry_a in stats_a['rx']:
        lid = entry_a['lid']
        rx_mpdus = entry_a['rx-num-mpdus']
        tx_mpdus = None
        for entry_b in stats_b['tx']:
            if entry_b['lid'] == entry_a['lid']:
                tx_mpdus = entry_b['tx-num-mpdus']
        if tx_mpdus is None:
            print '    RX LINK {0} *** missing matching TX section of {1} ***'.format(lid, mac_b)
            continue
        delta = rx_mpdus - tx_mpdus
        delta_percent = (100. / tx_mpdus) * delta
        print '    RX - LINK ID {0}    RECEIVED MPDUS: {1:8d} OTHER - SEND MPDUS: {2:8d} RX-TX-DELTA:    {3:8d} ({4:.2f}%)'.format(lid, rx_mpdus, tx_mpdus, delta, delta_percent)

    for entry in stats_a['tx']:
        lid = entry['lid']
        mpdus = entry['tx-num-mpdus']
        sacks = entry['tx-num-sacks']
        send_ack_delta = sacks - mpdus
        send_ack_delta_percent = (100. / mpdus) * (sacks - mpdus)
        print '    TX - LINK ID {0}    SEND MPDUS:     {1:8d} ACKED MPDUS:        {2:8d} SEND-ACK-DELTA: {3:8d} ({4:.2f}%)'.format(lid, mpdus, sacks, send_ack_delta, send_ack_delta_percent)


if __name__ == '__main__':

    import argparse

    # sudo python bin/hpav_stats.py --network-id ee:d0:71:7e:82:9e:06 --sta-a-mac f4:06:8d:15:71:56 --sta-b-mac f4:06:8d:15:6d:54 --interface eth2

    parser = argparse.ArgumentParser(description='Monitor HPAV link stats for two stations')
    parser.add_argument('--interface', type=str, help='network interface used to communicate with the HPAV devices', required=True)
    parser.add_argument('--network-id', type=str, help='network id to request the link stats for', required=True)
    parser.add_argument('--sta-a-mac', type=str, help='MAC address of first station', required=True)
    parser.add_argument('--sta-b-mac', type=str, help='MAC address of second station', required=True)
    args = parser.parse_args()

    HS = HPAVStats(HPAV_BINARY_PATH)

    link_stats_a = HS.link_stats(args.interface, args.network_id, args.sta_a_mac)
    link_stats_b = HS.link_stats(args.interface, args.network_id, args.sta_b_mac)

    print_link_stats(args.sta_a_mac, link_stats_a, args.sta_b_mac, link_stats_b)
    print_link_stats(args.sta_b_mac, link_stats_b, args.sta_a_mac, link_stats_a)
