package com.generalrobotix.ui.grxui;

import org.eclipse.jface.action.IContributionItem;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.ui.IWorkbenchActionConstants;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.actions.ActionFactory;
import org.eclipse.ui.actions.ContributionItemFactory;
import org.eclipse.ui.actions.ActionFactory.IWorkbenchAction;
import org.eclipse.ui.application.ActionBarAdvisor;
import org.eclipse.ui.application.IActionBarConfigurer;

import com.generalrobotix.ui.util.MessageBundle;


public class ApplicationActionBarAdvisor extends ActionBarAdvisor {
	private IWorkbenchAction exitAction;
	private IContributionItem showViewItem;
	private IWorkbenchAction preferencesAction;
	private IWorkbenchAction resetAction;

	public ApplicationActionBarAdvisor(IActionBarConfigurer configurer) {
		super(configurer);
	}

	protected void makeActions(final IWorkbenchWindow window) {
		exitAction = ActionFactory.QUIT.create(window);
		register(exitAction);
		showViewItem = ContributionItemFactory.VIEWS_SHORTLIST.create(window);
		preferencesAction = ActionFactory.PREFERENCES.create(window);
		register(preferencesAction);
		resetAction = ActionFactory.RESET_PERSPECTIVE.create(window);
		register(resetAction);
	}

	protected void fillMenuBar(IMenuManager menuBar) {
		MenuManager fileMenu = new MenuManager(MessageBundle.get("ApplicationActionBarAdvisor.menu.file"), //$NON-NLS-1$
				IWorkbenchActionConstants.M_FILE);
		menuBar.add(fileMenu);
		fileMenu.add(exitAction);
		MenuManager windowMenu = new MenuManager(MessageBundle.get("ApplicationActionBarAdvisor.menu.window"), //$NON-NLS-1$
				IWorkbenchActionConstants.M_WINDOW);
		
		menuBar.add(windowMenu);
		MenuManager showViewMenu = new MenuManager(MessageBundle.get("ApplicationActionBarAdvisor.menu.view")); //$NON-NLS-1$
	    showViewMenu.add(showViewItem);
	    windowMenu.add(showViewMenu);
	    windowMenu.add(preferencesAction);
	    windowMenu.add(resetAction);

	}

}
