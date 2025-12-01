import { createNativeBottomTabNavigator } from '@react-navigation/bottom-tabs/unstable';
import { NavigationContainer, useNavigation } from '@react-navigation/native';
import { createStackNavigator } from '@react-navigation/stack';
import type { FC } from 'react';
import React from 'react';
import {
  FlatList,
  ListRenderItem,
  Pressable,
  StyleSheet,
  Text,
  View,
} from 'react-native';
import { GestureHandlerRootView } from 'react-native-gesture-handler';
import { runOnUISync } from 'react-native-worklets';

import { Spacer } from './components';
import Container from './components/Container';
import { demos, DemoScreen } from './demos';
import { Example, Examples, MainStackProps } from './examples';
import { colors, layout } from './styles';

const Stack = createStackNavigator();

const ItemSeparatorComponent = () => <Spacer.Vertical size={16} />;

const ItemSeparatorComponent2 = () => (
  <View>
    <Spacer.Vertical size={12} />
    <View style={styles.hr} />
    <Spacer.Vertical size={12} />
  </View>
);

const ExamplesScreen: FC = () => {
  const navigation = useNavigation<MainStackProps>();

  const renderItem: ListRenderItem<Example> = ({
    item: { Icon, key, subtitle, title },
  }) => (
    <Pressable
      key={key}
      style={styles.buttonSmall}
      onPress={() => navigation.navigate(key)}
    >
      {/* <Icon /> */}
      <Text style={styles.subtitleSmall}>{subtitle}</Text>
      <Text style={styles.titleSmall}>{title}</Text>
    </Pressable>
  );

  return (
    <Container>
      <FlatList
        data={Examples}
        renderItem={renderItem}
        keyExtractor={(item) => item.key}
        contentContainerStyle={styles.scrollView}
        ItemSeparatorComponent={ItemSeparatorComponent2}
      />
    </Container>
  );
};

const DemoAppsScreen: FC = () => {
  const navigation = useNavigation<MainStackProps>();

  const renderItem: ListRenderItem<DemoScreen> = ({ item }) => (
    <Pressable
      onPress={() => navigation.navigate(item.key)}
      key={item.key}
      style={({ pressed }) => [
        styles.button,
        { borderStyle: pressed ? 'solid' : 'dashed' },
      ]}
    >
      <Text style={styles.title}>{item.title}</Text>
      <Text style={styles.subtitle}>{item.subtitle}</Text>
    </Pressable>
  );

  return (
    <Container>
      <FlatList
        data={demos}
        renderItem={renderItem}
        keyExtractor={(item) => item.key}
        contentContainerStyle={styles.scrollView}
        ItemSeparatorComponent={ItemSeparatorComponent}
      />
    </Container>
  );
};

const BenchmarksScreen: FC = () => {
  return <Container />;
};

const MainTabs = createNativeBottomTabNavigator<MainStackProps>({
  screens: {
    Examples: ExamplesScreen,
    DemoApps: DemoAppsScreen,
    Benchmarks: BenchmarksScreen,
  },
});

const MainTabsScreen: FC = () => {
  return (
    <MainTabs.Navigator>
      <MainTabs.Screen
        name="Examples"
        component={ExamplesScreen}
        options={{
          title: 'Examples',
          tabBarIcon: {
            type: 'sfSymbol',
            name: 'list.number',
          },
        }}
      />
      <MainTabs.Screen
        name="DemoApps"
        component={DemoAppsScreen}
        options={{
          title: 'Demo Apps',
          tabBarIcon: {
            type: 'sfSymbol',
            name: 'waveform.path',
          },
        }}
      />
      <MainTabs.Screen
        name="Benchmarks"
        component={BenchmarksScreen}
        options={{
          title: 'Benchmarks',

          tabBarIcon: {
            type: 'sfSymbol',
            name: 'gauge',
          },
        }}
      />
    </MainTabs.Navigator>
  );
};

const App: FC = () => {
  return (
    <GestureHandlerRootView style={styles.container}>
      <NavigationContainer>
        <Stack.Navigator
          screenOptions={{
            headerShown: true,
            headerTransparent: true,
            headerStyle: {
              backgroundColor: 'transparent',
            },
            headerTintColor: colors.white,
            headerBackTitle: 'Back',
            headerBackAccessibilityLabel: 'Go back',
          }}
        >
          <Stack.Screen
            name="MainTabs"
            component={MainTabsScreen}
            options={{ headerShown: false }}
          />
          {Examples.map((item) => (
            <Stack.Screen
              key={item.key}
              name={item.key}
              component={item.screen}
              options={{ title: item.title }}
            />
          ))}
          {demos.map((item) => (
            <Stack.Screen
              key={item.key}
              name={item.key}
              component={item.screen}
              options={{ title: item.title }}
            />
          ))}
        </Stack.Navigator>
      </NavigationContainer>
    </GestureHandlerRootView>
  );
};

export default App;

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  title: {
    fontSize: 24,
    fontWeight: '700',
    color: colors.white,
  },
  subtitle: {
    opacity: 0.6,
    color: colors.white,
  },
  button: {
    borderWidth: 2,
    borderColor: colors.border,
    borderRadius: layout.radius,
    paddingVertical: layout.spacing * 2,
    paddingHorizontal: layout.spacing * 2,
  },
  buttonSmall: {
    paddingVertical: layout.spacing,
    paddingHorizontal: layout.spacing * 2,
  },
  titleSmall: {
    fontSize: 18,
    fontWeight: '600',
    color: colors.white,
  },
  subtitleSmall: {
    opacity: 0.6,
    color: colors.white,
  },
  scrollView: {},
  hr: {
    height: StyleSheet.hairlineWidth,
    backgroundColor: colors.border,
    marginHorizontal: 64,
  },
});
