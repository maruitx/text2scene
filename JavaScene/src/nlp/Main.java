package nlp;

import edu.stanford.nlp.semgraph.SemanticGraphFactory;
import edu.stanford.nlp.simple.Sentence;

import java.util.List;
import java.util.Optional;

public class Main {

    public static void main(String[] args) {
        Sentence s = new Sentence("I am going to go to the store tomorrow");
        List<Optional<String>> labels = s.incomingDependencyLabels(SemanticGraphFactory.Mode.ENHANCED_PLUS_PLUS);
        List<Optional<Integer>> governors = s.governors(SemanticGraphFactory.Mode.ENHANCED_PLUS_PLUS);
        for(int i = 0; i < labels.size(); i++) {
            int govIndex = governors.get(i).get();
            String govLabel = "ROOT";
            if(govIndex != -1) govLabel = s.word(govIndex);
            System.out.println(labels.get(i).get() + ": " + govLabel + ", " + s.word(i));
        }
    }
}
