/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.nfc;

import android.annotation.Nullable;
import android.nfc.NdefMessage;
import android.os.Bundle;

import java.io.IOException;

public interface DeviceHost {
    public interface DeviceHostListener {
        public void onRemoteEndpointDiscovered(TagEndpoint tag);

/* START [P150204001] - Enable WCE with UICC/eSE. */
        /**
         * Notifies transaction
         */
        public void onCardEmulationDeselected();

        /**
         * Notifies transaction
         */
        public void onCardEmulationAidSelected(byte[] aid, byte[] data, int evtSrc);
/* END [P150204001] - Enable WCE with UICC/eSE. */
        public void onHostCardEmulationActivated();
        public void onHostCardEmulationData(byte[] data);
        public void onHostCardEmulationDeactivated();

        /**
         * Notifies P2P Device detected, to activate LLCP link
         */
        public void onLlcpLinkActivated(NfcDepEndpoint device);

        /**
         * Notifies P2P Device detected, to activate LLCP link
         */
        public void onLlcpLinkDeactivated(NfcDepEndpoint device);

        public void onLlcpFirstPacketReceived(NfcDepEndpoint device);

        public void onRemoteFieldActivated();

        public void onRemoteFieldDeactivated();

/* START [P150204001] - Enable WCE with UICC/eSE. */
        /**
         * Notifies that the SE has been activated in listen mode
         */
        public void onSeListenActivated();

        /**
         * Notifies that the SE has been deactivated
         */
        public void onSeListenDeactivated();

        public void onSeApduReceived(byte[] apdu);

        public void onSeEmvCardRemoval();

        public void onSeMifareAccess(byte[] block);
/* END [P150204001] - Enable WCE with UICC/eSE. */
    }

    public interface TagEndpoint {
        boolean connect(int technology);
        boolean reconnect();
        boolean disconnect();

        boolean presenceCheck();
        boolean isPresent();
        void startPresenceChecking(int presenceCheckDelay,
                                   @Nullable TagDisconnectedCallback callback);

        int[] getTechList();
        void removeTechnology(int tech); // TODO remove this one
        Bundle[] getTechExtras();
        byte[] getUid();
        int getHandle();

        byte[] transceive(byte[] data, boolean raw, int[] returnCode);

        boolean checkNdef(int[] out);
        byte[] readNdef();
        boolean writeNdef(byte[] data);
        NdefMessage findAndReadNdef();
        boolean formatNdef(byte[] key);
        boolean isNdefFormatable();
        boolean makeReadOnly();

        int getConnectedTechnology();
    }

    public interface TagDisconnectedCallback {
        void onTagDisconnected(long handle);
    }

    public interface NfceeEndpoint {
        // TODO flesh out multi-EE and use this
    }

    public interface NfcDepEndpoint {

        /**
         * Peer-to-Peer Target
         */
        public static final short MODE_P2P_TARGET = 0x00;
        /**
         * Peer-to-Peer Initiator
         */
        public static final short MODE_P2P_INITIATOR = 0x01;
        /**
         * Invalid target mode
         */
        public static final short MODE_INVALID = 0xff;

        public byte[] receive();

        public boolean send(byte[] data);

        public boolean connect();

        public boolean disconnect();

        public byte[] transceive(byte[] data);

        public int getHandle();

        public int getMode();

        public byte[] getGeneralBytes();

        public byte getLlcpVersion();
    }

    public interface LlcpSocket {
        public void connectToSap(int sap) throws IOException;

        public void connectToService(String serviceName) throws IOException;

        public void close() throws IOException;

        public void send(byte[] data) throws IOException;

        public int receive(byte[] recvBuff) throws IOException;

        public int getRemoteMiu();

        public int getRemoteRw();

        public int getLocalSap();

        public int getLocalMiu();

        public int getLocalRw();
    }

    public interface LlcpServerSocket {
        public LlcpSocket accept() throws IOException, LlcpException;

        public void close() throws IOException;
    }

    public interface LlcpConnectionlessSocket {
        public int getLinkMiu();

        public int getSap();

        public void send(int sap, byte[] data) throws IOException;

        public LlcpPacket receive() throws IOException;

        public void close() throws IOException;
    }

    /**
     * Called at boot if NFC is disabled to give the device host an opportunity
     * to check the firmware version to see if it needs updating. Normally the firmware version
     * is checked during {@link #initialize(boolean enableScreenOffSuspend)},
     * but the firmware may need to be updated after an OTA update.
     *
     * <p>This is called from a thread
     * that may block for long periods of time during the update process.
     */
    public void checkFirmware();

    public boolean initialize();

    public boolean deinitialize();

    public String getName();

    public void enableDiscovery(NfcDiscoveryParameters params, boolean restart);

    public void disableDiscovery();

    public boolean sendRawFrame(byte[] data);

    public boolean routeAid(byte[] aid, int route, int power);

    public boolean unrouteAid(byte[] aid);

/* START [P160421001] - Patch for Dynamic SE Selection */
    public boolean setRoutingEntry(int type, int value, int route, int power);

    public boolean clearRoutingEntry(int type);
	
	public void doSetDefaultSe(int defaultSeId);
    public int doGetDefaultSe();
/* END [P160421001] - Patch for Dynamic SE Selection */

/* START [P161006001] - Configure Default SecureElement and LMRT */
    public void SetDefaultRoute(int defaultSeId);
/* END [P161006001] - Configure Default SecureElement and LMRT */

    public boolean commitRouting();

    public void registerT3tIdentifier(byte[] t3tIdentifier);

    public void deregisterT3tIdentifier(byte[] t3tIdentifier);

    public void clearT3tIdentifiersCache();

    public int getLfT3tMax();

/* START [P1504130001] - Support SE on Device Power Off State */
    public void doSetScreenOrPowerState(int state);
/* END [P1504130001] - Support SE on Device Power Off State */

/* START [P1604040001] - Support Dual-SIM solution */
    public void doSetPreferredSimSlot(int preferredSim);
/* END [P1604040001] - Support Dual-SIM solution */

    public LlcpConnectionlessSocket createLlcpConnectionlessSocket(int nSap, String sn)
            throws LlcpException;

    public LlcpServerSocket createLlcpServerSocket(int nSap, String sn, int miu,
            int rw, int linearBufferLength) throws LlcpException;

    public LlcpSocket createLlcpSocket(int sap, int miu, int rw,
            int linearBufferLength) throws LlcpException;

    public boolean doCheckLlcp();

    public boolean doActivateLlcp();

    public void resetTimeouts();

    public boolean setTimeout(int technology, int timeout);

    public int getTimeout(int technology);

    public void doAbort();

    boolean canMakeReadOnly(int technology);

    int getMaxTransceiveLength(int technology);

    void setP2pInitiatorModes(int modes);

    void setP2pTargetModes(int modes);

    boolean getExtendedLengthApdusSupported();

/* START [P150204001] - Enable WCE with UICC/eSE. */
    byte[][] getWipeApdus();
/* END [P150204001] - Enable WCE with UICC/eSE. */

    int getDefaultLlcpMiu();

    int getDefaultLlcpRwSize();

    String dump();

    boolean enableScreenOffSuspend();

    boolean disableScreenOffSuspend();
    // START [J00000004] System LSI - Flip Cover
    public byte[] transceiveAuthData(byte[] data);
    public byte[] startCoverAuth();
    public boolean stopCoverAuth();
    // END [J00000004] System LSI - Flip Cover
}
