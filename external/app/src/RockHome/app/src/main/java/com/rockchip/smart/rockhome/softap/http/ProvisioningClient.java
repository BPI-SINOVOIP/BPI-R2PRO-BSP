/**
 * Copyright 2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * You may not use this file except in compliance with the License. A copy of the License is located the "LICENSE.txt"
 * file accompanying this source. This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language governing permissions and limitations
 * under the License.
 */
package com.rockchip.smart.rockhome.softap.http;

import android.content.Context;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import com.rockchip.smart.rockhome.R;
import com.rockchip.smart.rockhome.WifiInfo;

import org.apache.commons.io.IOUtils;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.Socket;
import java.net.URL;
import java.net.UnknownHostException;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.util.ArrayList;
import java.util.List;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManagerFactory;

public class ProvisioningClient {
    private static final String TAG = "ProvisioningClient";
    private String endpoint;
    private SSLSocketFactory pinnedSSLSocketFactory;

    private static ProvisioningClient mClient;

    public static ProvisioningClient getProvisioningClient(Context context) {
        if (mClient == null) {
            try {
                mClient = new ProvisioningClient(context);
                mClient.setEndpoint("http://10.201.126.1:8443");
            } catch (Exception e) {

            }
        }
        return mClient;
    }

    private ProvisioningClient(Context context) throws Exception {
        this.pinnedSSLSocketFactory = getPinnedSSLSocketFactory(context);
    }

    public void setEndpoint(String endpoint) {
        this.endpoint = endpoint;
    }

    public boolean isWifiConnected() {
        Log.d(TAG, "check isWifiConnected");
        try {
            DeviceContextInfo info = getDeviceContextInfo();
            return !info.getIp().equals("127.0.0.1");
        } catch (JSONException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return false;
    }

    public DeviceContextInfo getDeviceContextInfo() throws JSONException, IOException {
        URL companionInfoEndpoint = new URL(endpoint + "/provision/deviceContext");

        HttpURLConnection connection = (HttpURLConnection) companionInfoEndpoint.openConnection();

        JSONObject request = doRequest(connection);

        String ssid = request.getString("ssid");
        String ip = request.getString("ip");
        boolean authorized = request.getBoolean("authorized");

        return new DeviceContextInfo(ssid, ip, authorized);
    }

    public List<WifiInfo> getWifiListInfo() throws JSONException, IOException {
        List<WifiInfo> result = new ArrayList<WifiInfo>();
        URL companionInfoEndpoint = new URL(endpoint + "/provision/wifiListInfo");
        HttpURLConnection connection = (HttpURLConnection) companionInfoEndpoint.openConnection();

        JSONObject response = doRequest(connection);
        if (response == null) {
            Log.d(TAG, "Get wifi list, response is null.");
            return result;
        }

        JSONArray array = response.getJSONArray("content");
        if (array == null) {
            return result;
        }

        JSONObject obj = null;
        WifiInfo info = null;
        int length = array.length();

        for (int i=0; i<length; i++) {
            obj = array.getJSONObject(i);
            info = new WifiInfo();

            info.setBssid(obj.getString(WifiInfo.BSSID));
            info.setFlags(obj.getString(WifiInfo.FLAGS));
            info.setFrequency(obj.getString(WifiInfo.FREQUENCY));
            info.setSignalLevel(obj.getString(WifiInfo.SIGNALLEVEL));
            info.setSsid(obj.getString(WifiInfo.SSID));
            result.add(info);
        }
        return result;
    }

    public boolean postWifiSetupInfo(String ssid, String pwd) {
        try {
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("ssid", ssid);
            jsonObject.put("pwd", pwd);

            Log.d(TAG, jsonObject.toString());
            URL companionInfoEndpoint = new URL(endpoint + "/provision/wifiSetup");
            HttpURLConnection connection = (HttpURLConnection) companionInfoEndpoint.openConnection();
            return doRequestForResult(connection, jsonObject.toString());
        } catch (JSONException | IOException e ) {
            e.printStackTrace();
            Log.d(TAG, Log.getStackTraceString(e));
        }
        return false;
    }

    public boolean postConnectResult(boolean result) {
        try {
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("result", result ? "1" : "0");

            Log.d(TAG, jsonObject.toString());
            URL companionInfoEndpoint = new URL(endpoint + "/provision/connectResult");
            HttpURLConnection connection = (HttpURLConnection) companionInfoEndpoint.openConnection();
            return doRequestForResult(connection, jsonObject.toString());
        } catch (JSONException | IOException e ) {
            e.printStackTrace();
            Log.d(TAG, Log.getStackTraceString(e));
        }
        return false;
    }

    public boolean checkWifiState() {
        try {
            URL companionInfoEndpoint = new URL(endpoint + "/provision/wifiState");
            HttpURLConnection connection = (HttpURLConnection) companionInfoEndpoint.openConnection();
            String response = doRequestGetString(connection);
            Log.d("WifiFragment", "checkWifiState " + response);

            if (response.equals("1")) {
                return true;
            }
        } catch (JSONException | IOException e) {
            e.printStackTrace();
        }
        return false;
    }

    JSONObject doRequest(HttpURLConnection connection) throws IOException, JSONException {
        return doRequest(connection, null);
    }

    JSONObject doRequest(HttpURLConnection connection, String data) throws IOException, JSONException {
        int responseCode = -1;
        InputStream response = null;
        DataOutputStream outputStream = null;

        try {
            if (connection instanceof HttpsURLConnection) {
                ((HttpsURLConnection) connection).setSSLSocketFactory(pinnedSSLSocketFactory);
            }

            connection.setRequestProperty("Content-Type", "application/json");
            if (data != null) {
                connection.setRequestMethod("POST");
                connection.setDoOutput(true);

                outputStream = new DataOutputStream(connection.getOutputStream());
                outputStream.write(data.getBytes());
                outputStream.flush();
                outputStream.close();
            } else {
                connection.setRequestMethod("GET");
            }

            responseCode = connection.getResponseCode();
            Log.d(TAG, "responseCode:" + responseCode);
            response = connection.getInputStream();

            if (responseCode != 204) {
                String responseString = IOUtils.toString(response);
                if (TextUtils.isEmpty(responseString))
                    return  null;
                JSONObject jsonObject = new JSONObject(responseString);
                return jsonObject;
            } else {
                return null;
            }
        } catch (IOException e) {
            if (responseCode < 200 || responseCode >= 300) {
                response = connection.getErrorStream();
                if (response != null) {
                    String responseString = IOUtils.toString(response);
                    //throw new RuntimeException(responseString);
                }
            }
            throw e;
        } finally {
            IOUtils.closeQuietly(outputStream);
            IOUtils.closeQuietly(response);
        }
    }

    String doRequestGetString(HttpURLConnection connection) throws IOException, JSONException {
        int responseCode = -1;
        InputStream response = null;
        DataOutputStream outputStream = null;

        try {
            if (connection instanceof HttpsURLConnection) {
                ((HttpsURLConnection) connection).setSSLSocketFactory(pinnedSSLSocketFactory);
            }

            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestMethod("GET");

            responseCode = connection.getResponseCode();
            Log.d(TAG, "responseCode:" + responseCode);
            response = connection.getInputStream();

            if (responseCode != 204) {
                String responseString = IOUtils.toString(response);
                if (TextUtils.isEmpty(responseString))
                    return  "";

                return responseString;
            } else {
                return "";
            }
        } catch (IOException e) {
            if (responseCode < 200 || responseCode >= 300) {
                response = connection.getErrorStream();
                if (response != null) {
                    String responseString = IOUtils.toString(response);
                    //throw new RuntimeException(responseString);
                }
            }
            throw e;
        } finally {
            IOUtils.closeQuietly(outputStream);
            IOUtils.closeQuietly(response);
        }
    }

    boolean doRequestForResult(HttpURLConnection connection, String data) throws IOException, JSONException {
        int responseCode = -1;
        InputStream response = null;
        DataOutputStream outputStream = null;

        try {
            if (connection instanceof HttpsURLConnection) {
                ((HttpsURLConnection) connection).setSSLSocketFactory(pinnedSSLSocketFactory);
            }

            connection.setRequestProperty("Content-Type", "application/json");
            if (data != null) {
                connection.setRequestMethod("POST");
                connection.setDoOutput(true);

                outputStream = new DataOutputStream(connection.getOutputStream());
                outputStream.write(data.getBytes());
                outputStream.flush();
                outputStream.close();
            } else {
                connection.setRequestMethod("GET");
            }

            responseCode = connection.getResponseCode();
            Log.d(TAG, "responseCode:" + responseCode);
            response = connection.getInputStream();

            if (responseCode == 200) {
                return true;
            }
        } catch (IOException e) {

        } finally {
            IOUtils.closeQuietly(outputStream);
            IOUtils.closeQuietly(response);
        }
        return false;
    }

    private SSLSocketFactory getPinnedSSLSocketFactory(Context context) throws Exception {
        InputStream caCertInputStream = null;
        try {
            caCertInputStream = context.getResources().openRawResource(R.raw.ca);
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            Certificate caCert = cf.generateCertificate(caCertInputStream);

            KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());
            trustStore.load(null, null);
            trustStore.setCertificateEntry("myca", caCert);

            TrustManagerFactory trustManagerFactory =
                    TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            trustManagerFactory.init(trustStore);

            SSLContext sslContext = SSLContext.getInstance("TLS");
            sslContext.init(null, trustManagerFactory.getTrustManagers(), null);

            if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
                return new TLSSocketFactory(sslContext.getSocketFactory());
            }

            return sslContext.getSocketFactory();
        } finally {
            IOUtils.closeQuietly(caCertInputStream);
        }
    }

    private SSLSocketFactory getPinnedSSLSocketFactory(String crt) throws Exception {
        InputStream caCertInputStream = null;
        try {
            caCertInputStream = new ByteArrayInputStream(crt.getBytes("UTF-8"));
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            Certificate caCert = cf.generateCertificate(caCertInputStream);

            KeyStore trustStore = KeyStore.getInstance(KeyStore.getDefaultType());
            trustStore.load(null, null);
            trustStore.setCertificateEntry("myca", caCert);

            TrustManagerFactory trustManagerFactory =
                    TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            trustManagerFactory.init(trustStore);

            SSLContext sslContext = SSLContext.getInstance("TLS");
            sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
            return sslContext.getSocketFactory();
        } finally {
            IOUtils.closeQuietly(caCertInputStream);
        }
    }

    public static class TLSSocketFactory extends SSLSocketFactory {

        private SSLSocketFactory internalSSLSocketFactory;

        public TLSSocketFactory(SSLSocketFactory delegate) throws KeyManagementException, NoSuchAlgorithmException {
            internalSSLSocketFactory = delegate;
        }

        @Override
        public String[] getDefaultCipherSuites() {
            return internalSSLSocketFactory.getDefaultCipherSuites();
        }

        @Override
        public String[] getSupportedCipherSuites() {
            return internalSSLSocketFactory.getSupportedCipherSuites();
        }

        @Override
        public Socket createSocket(Socket s, String host, int port, boolean autoClose) throws IOException {
            return enableTLSOnSocket(internalSSLSocketFactory.createSocket(s, host, port, autoClose));
        }

        @Override
        public Socket createSocket(String host, int port) throws IOException, UnknownHostException {
            return enableTLSOnSocket(internalSSLSocketFactory.createSocket(host, port));
        }

        @Override
        public Socket createSocket(String host, int port, InetAddress localHost, int localPort) throws IOException, UnknownHostException {
            return enableTLSOnSocket(internalSSLSocketFactory.createSocket(host, port, localHost, localPort));
        }

        @Override
        public Socket createSocket(InetAddress host, int port) throws IOException {
            return enableTLSOnSocket(internalSSLSocketFactory.createSocket(host, port));
        }

        @Override
        public Socket createSocket(InetAddress address, int port, InetAddress localAddress, int localPort) throws IOException {
            return enableTLSOnSocket(internalSSLSocketFactory.createSocket(address, port, localAddress, localPort));
        }

        /*
        * Utility methods
        */
        private static Socket enableTLSOnSocket(Socket socket) {
            if (socket != null && (socket instanceof SSLSocket)
                    && isTLSServerEnabled((SSLSocket) socket)) { // skip the fix if server doesn't provide the TLS version
                ((SSLSocket) socket).setEnabledProtocols(new String[]{"TLSv1.2"});
            }
            return socket;
        }

        private static boolean isTLSServerEnabled(SSLSocket sslSocket) {
            for (String protocol : sslSocket.getSupportedProtocols()) {
                if (protocol.equals("TLSv1.2")) {
                    return true;
                }
            }
            return false;
        }
    }
}
