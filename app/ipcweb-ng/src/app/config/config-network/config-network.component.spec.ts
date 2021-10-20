import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigNetworkComponent } from './config-network.component';

describe('ConfigNetworkComponent', () => {
  let component: ConfigNetworkComponent;
  let fixture: ComponentFixture<ConfigNetworkComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConfigNetworkComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigNetworkComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
