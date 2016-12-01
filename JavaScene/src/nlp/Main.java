package nlp;

import edu.stanford.nlp.semgraph.SemanticGraphFactory;
import edu.stanford.nlp.simple.Document;
import edu.stanford.nlp.simple.Sentence;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.List;
import java.util.Optional;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.PrintWriter;

public class Main {

    private static class DocumentParser extends Thread {
        private Socket socket;
        private int clientNumber;

        public DocumentParser(Socket socket, int clientNumber) {
            this.socket = socket;
            this.clientNumber = clientNumber;
            log("New connection with client# " + clientNumber + " at " + socket);
        }

        public static String processDocument(String text)
        {
            StringBuilder result = new StringBuilder();

            Document d = new Document(text);
            List<Sentence> sentences = d.sentences();
            for(Sentence s : sentences)
            {
                result.append(s.text());
                result.append('|');

                List<Optional<String>> labels = s.incomingDependencyLabels(SemanticGraphFactory.Mode.ENHANCED_PLUS_PLUS);
                List<Optional<Integer>> governors = s.governors(SemanticGraphFactory.Mode.ENHANCED_PLUS_PLUS);

                for(int i = 0; i < labels.size(); i++) {
                    int govIndex = governors.get(i).get();
                    String govLabel = "ROOT";
                    if(govIndex != -1) govLabel = s.word(govIndex);
                    //System.out.println(labels.get(i).get() + ": " + govLabel + ", " + s.word(i));
                    result.append(labels.get(i).get() + "@" + govLabel + "@" + s.word(i) + "^");
                }
            }
            return result.toString();
        }

        /**
         * Services this thread's client by first sending the
         * client a welcome message then repeatedly reading strings
         * and sending back the capitalized version of the string.
         */
        public void run() {
            try {

                // Decorate the streams so we can send characters
                // and not just bytes.  Ensure output is flushed
                // after every newline.
                BufferedReader in = new BufferedReader(
                        new InputStreamReader(socket.getInputStream()));
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

                // Send a welcome message to the client.
                //out.println("Connection-successful-" + clientNumber);

                // Process a single incoming document.
                String input = in.readLine();
                out.println(processDocument(input));
            } catch (IOException e) {
                log("Error handling client# " + clientNumber + ": " + e);
            } finally {
                try {
                    socket.close();
                } catch (IOException e) {
                    log("Couldn't close a socket, what's going on?");
                }
                log("Connection with client# " + clientNumber + " closed");
            }
        }

        /**
         * Logs a simple message.  In this case we just write the
         * message to the server applications standard output.
         */
        private void log(String message) {
            System.out.println(message);
        }
    }

    public static void runNLPServer() throws IOException
    {
        System.out.println("The NLP server is running.");
        int clientNumber = 0;
        ServerSocket listener = new ServerSocket(3000);
        try {
            while (true) {
                new DocumentParser(listener.accept(), clientNumber++).start();
            }
        } finally {
            listener.close();
        }
    }

    public static void main(String[] args) throws IOException {
        runNLPServer();
        //String result = processDocument("I am going to go to the store tomorrow. What should I purchase?");
        //System.out.println(result);
        /*Sentence s = new Sentence("I am going to go to the store tomorrow");
        List<Optional<String>> labels = s.incomingDependencyLabels(SemanticGraphFactory.Mode.ENHANCED_PLUS_PLUS);
        List<Optional<Integer>> governors = s.governors(SemanticGraphFactory.Mode.ENHANCED_PLUS_PLUS);
        for(int i = 0; i < labels.size(); i++) {
            int govIndex = governors.get(i).get();
            String govLabel = "ROOT";
            if(govIndex != -1) govLabel = s.word(govIndex);
            System.out.println(labels.get(i).get() + ": " + govLabel + ", " + s.word(i));
        }*/
    }
}
